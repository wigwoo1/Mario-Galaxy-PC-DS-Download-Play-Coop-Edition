#define WIN32_LEAN_AND_MEAN
#include "assets.hpp"
#include <JSystem/JAudio2/JAUAudioArcInterpreter.hpp>
#include <JSystem/JAudio2/JAUSoundTable.hpp>
#include <JSystem/JAudio2/JAUStdSoundInfo.hpp>
#include <JSystem/JAudio2/JAISoundChild.hpp>
#include <Native/HeapPlacement.hpp>
#include <Native/DspAddressSpace.hpp>
#include <Native/DspAudioRuntime.hpp>
#include <Native/DspMailbox.hpp>
#include <Native/JasStreamEndian.hpp>
#include <JSystem/JAudio2/JASAiCtrl.hpp>
#include <JSystem/JAudio2/JASAudioThread.hpp>
#include <JSystem/JAudio2/JASChannel.hpp>
#include <JSystem/JAudio2/JASDSPChannel.hpp>
#include <JSystem/JAudio2/JASDriverIF.hpp>
#include <JSystem/JAudio2/JASHeapCtrl.hpp>
#include <JSystem/JAudio2/JASAramStream.hpp>
#include <JSystem/JAudio2/JASDvdThread.hpp>
#include <JSystem/JAudio2/JAIStreamMgr.hpp>
#include <JSystem/JAudio2/JAUStreamDataMgr.hpp>
#include <JSystem/JAudio2/JAUStreamFileTable.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <revolution/ai.h>
#include <revolution/dsp.h>
#include "native_jkr_aram_port.hpp"
#include "native_dvd_port.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

namespace {
std::mutex mutex;
std::condition_variable changed;
OSThread* mainThread=nullptr;
struct Catalog : JAUAudioArcInterpreter {
    const u8* bst=nullptr;u32 bytes=0;unsigned bsft=0,other=0;
    void readBST(const void* p,u32 n) override {assert(!bst);bst=static_cast<const u8*>(p);bytes=n;}
    void readBSFT(const void*) override {++bsft;}
    void readWS(u32,const void*,u32) override {++other;}
    void readBNK(u32,const void*) override {++other;}
    void readBSC(const void*,u32) override {++other;}
    void readBSTN(const void*,u32) override {++other;}
    void readBMS(u32,const void*,u32) override {++other;}
    void readBMS_fromArchive(u32) override {++other;}
    void newVoiceBank(u32,u32) override {++other;}
    void newDynamicSeqBlock(u32) override {++other;}
    void readMaxSeCategory(int,int,int) override {++other;}
    void beginBNKList(u32,u32) override {++other;}
    void endBNKList() override {++other;}
    bool readCommandMore(u32) override {assert(false&&"Unexpected actual BAA command");return false;}
};
u32 verifyCatalog(JAUSoundTable& table,JAUStdSoundInfo& info,const u8* bytes,u32 length) {
    auto word=[&](u32 offset){extent(offset,4,length);return be32(bytes+offset);};
    const u32 root=word(12),sections=word(root);extent(root+4,u64(sections)*4,length);
    unsigned streamRecords=0,matches=0;u32 selected=0;
    for(u32 section=0;section<sections;++section) {
        const u32 so=word(root+4+section*4);if(!so)continue;
        const u32 groups=word(so);extent(so+4,u64(groups)*4,length);
        for(u32 group=0;group<groups;++group) {
            const u32 go=word(so+4+group*4);if(!go)continue;
            const u32 items=word(go);extent(go+8,u64(items)*4,length);
            for(u32 item=0;item<items;++item) {
                const u32 descriptor=word(go+8+item*4),offset=descriptor&0xffffff;
                if(!offset||(descriptor>>28)!=7)continue;
                extent(offset,8,length);const u32 nameOffset=word(offset+4);extent(nameOffset,1,length);
                assert(std::memchr(bytes+nameOffset,0,length-nameOffset));
                const char* path=reinterpret_cast<const char*>(bytes+nameOffset);
                const u32 id=(section<<24)|(group<<16)|item;const JAISoundID soundId(id);
                assert(table.getData(soundId)==reinterpret_cast<const JAUSoundTableItem*>(bytes+offset));
                assert((table.getTypeID(soundId)&0xf0)==0x70&&info.getSoundType(soundId)==2);
                assert(info.getCategory(soundId)==int(group)&&info.getPriority(soundId)==bytes[offset]);
                assert(!std::strcmp(info.getStreamFilePath(soundId),path));
                const int entry=DVDConvertPathToEntrynum(path);assert(entry>=0&&info.getStreamFileEntry(soundId)==entry);
                if(!std::strcmp(path,"/AudioRes/Stream/SMG_ev_prolo04_strm.ast")) {
                    ++matches;selected=id;
                    assert(bytes[offset]==128&&bytes[offset+1]==85&&((u16(bytes[offset+2])<<8)|bytes[offset+3])==14);
                }
                ++streamRecords;
            }
        }
    }
    assert(streamRecords==75&&matches==1&&selected==0x02000022);
    std::printf("PASS all%u actual BST stream records resolve through original JAUStdSoundInfo/native DVD; prologue ID=%08x volume85/255 channelCtrl14\n",streamRecords,selected);
    return selected;
}
struct Controller {
    JAUSoundTable table{true};
    JAUStdSoundInfo info{true};
    u32 soundId=0;
    JASHeap root; // Must outlive the static-ARAM manager's child heaps.
    JAUStreamStaticAramMgr aram;
    JAIStreamMgr manager{false};
    JAISoundHandle handle;

    unsigned phase=0,subframes=0,frames=0,dacFrames=0,maxPosition=0;
    unsigned nonzero=0,stereoDifference=0,stopFrame=0,pauseFrame=0;
    unsigned heldPosition=0,resumeNonzero=0,startNonzero=0,cycles=0,constantDacFrames=0;
    std::chrono::steady_clock::time_point naturalStart;
    unsigned naturalNonzero=0,naturalStereo=0;
    bool holdOutputExpected=false;
    s16 heldLeft=0,heldRight=0;
    Controller(const u8* bst,u32 bytes) {
        table.init(bst);table.nativeResourceSize=bytes;assert(table.isValid());
        soundId=verifyCatalog(table,info,bst,bytes);
        root.initRootHeap(reinterpret_cast<void*>(0x8000),0x300000);aram.alloc(36,&root);
        assert(aram.mSize==2);manager.setStreamDataMgr(&info);manager.setStreamAramMgr(&aram);
    }
    void start() {
        assert(manager.startSound(JAISoundID(soundId),&handle,nullptr));
        assert(handle.isSoundAttached()&&JAIStream::memPool_.getFreeMemCount()==2);
        auto* stream=handle.getSound()->asStream();assert(stream&&stream->mCategory==0);
        assert(std::fabs(stream->mParams.mProperty.mVolume-85.0f/255.0f)<.000001f);
        assert(stream->children[0]&&stream->children[1]);
        assert(stream->children[0]->mMove.mParams.mPan==0&&stream->children[1]->mMove.mParams.mPan==1);
        for(unsigned i=2;i<6;++i)assert(!stream->children[i]);
        assert(JAISoundChild::memPool_.getFreeMemCount()==158);
    }
    void drained() {
        assert(!manager.isActive()&&!handle.isSoundAttached()&&aram.mBits.none()&&JAISoundChild::memPool_.getFreeMemCount()==160);
        assert(JAIStream::memPool_.getFreeMemCount()==3&&JASChannel::memPool_.getFreeMemCount()==72&&JASDSPChannel::getNumFree()==64);
    }
    void held(JAIStream* stream) {
        assert(stream&&stream->mPrepareState==4&&stream->mIsPaused);
        assert(u32(stream->inner_.aramStream._0B8)==heldPosition);
        assert(stream->inner_.aramStream._0AE&1);
        assert(aram.mBits.count()==1&&JAIStream::memPool_.getFreeMemCount()==2&&JAISoundChild::memPool_.getFreeMemCount()==158);
        assert(JASChannel::memPool_.getFreeMemCount()==70&&JASDSPChannel::getNumFree()==62);
        int left=0,right=0;
        auto sine=[](unsigned index) {
            assert(index<128);const u32 word=JASDsp::DSPRES_FILTER[256+index/2];
            return s16(index&1?word&0xffff:word>>16);
        };
        for(unsigned i=0;i<2;++i) {
            auto* parent=stream->inner_.aramStream.mChannels[i];
            assert(parent&&parent->mDspCh&&parent->mDspCh->mChannel->mPauseFlag);
            const auto& voice=*parent->mDspCh->mChannel;
            // The pinned SMG DSP pause field selects the last constant sample.
            // Evaluate its front-only automatic-mixer output independently;
            // never assume silence or capture the output as its own reference.
            assert(voice._58==1&&voice._E==0&&voice._52==0&&(voice._50&0xff)==0&&voice._54==voice._56);
            const unsigned x=(voice._50>>8)&0x7f;
            const int front=sine(127);
            const int l=((int(sine(x^127))*front)>>15)*int(voice._54)>>15;
            const int r=((int(sine(x))*front)>>15)*int(voice._54)>>15;
            left+=(l*int(voice._66))>>16;right+=(r*int(voice._66))>>16;
        }
        // Original readDspBuffer calls imixcopy(secondPlane, firstPlane, ...).
        // Compare the actual original DAC order rather than DSP plane labels.
        heldLeft=s16(right);heldRight=s16(left);
    }
    void tick() {
        if(++subframes%7 || phase==7)return;
        ++frames;
        if(phase==0){manager.pause(false);start();startNonzero=nonzero;phase=1;}
        if(phase==8) {
            manager.pause(false);start();maxPosition=0;naturalNonzero=nonzero;naturalStereo=stereoDifference;
            naturalStart=std::chrono::steady_clock::now();phase=9;
        }
        manager.calc();manager.mixOut();
        auto* stream=handle.isSoundAttached()?handle.getSound()->asStream():nullptr;
        const unsigned position=stream?u32(stream->inner_.aramStream._0B8):0;
        maxPosition=std::max(maxPosition,position);
        if(phase==1&&stream&&stream->mPrepareState==4&&position>8000&&nonzero>startNonzero+5000) {
            handle->pause(true);pauseFrame=frames;phase=2;
        } else if(phase==2) {
            assert(stream);
            if(frames-pauseFrame>=6) {
                if(frames-pauseFrame==6)heldPosition=position;
                held(stream);holdOutputExpected=true;
            }
            if(frames-pauseFrame>=36) {
                // Clearing the sound pause must not override a manager pause.
                manager.pause(true);handle->pause(false);manager.mixOut();
                pauseFrame=frames;phase=3;
            }
        } else if(phase==3) {
            held(stream);assert(!handle->isPaused());
            if(frames-pauseFrame>=30) {
                manager.pause(false);manager.mixOut();holdOutputExpected=false;
                resumeNonzero=nonzero;phase=4;
            }
        } else if(phase==4&&stream&&position>heldPosition+4000&&nonzero>resumeNonzero+1000) {
            assert(!stream->mIsPaused);
            std::printf("PASS live sound/manager pause hierarchy and resume cycle%u: source held at%u, resumed to%u, held DAC chunks=%u; stream/ARAM/parents retained\n",cycles+1,heldPosition,position,constantDacFrames);
            manager.pause(true);pauseFrame=frames;phase=5;
        } else if(phase==5&&frames-pauseFrame>=6) {
            heldPosition=position;held(stream);manager.stop(10);stopFrame=frames;phase=6;
        } else if(phase==6) {
            assert(frames-stopFrame<120);
            if(!manager.isActive()) {
                drained();
                std::printf("PASS stop(10) while manager-paused drains after%u frames in cycle%u; all original pools/ARAM restored\n",frames-stopFrame,cycles+1);
                if(++cycles==3){phase=8;assert(constantDacFrames>=180);}
                else {manager.pause(false);phase=0;}
            }
        } else if(phase==9&&!manager.isActive()) {
            drained();const double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-naturalStart).count();
            const double nominal=841863.0/32000.0;
            assert(seconds>nominal*.95&&seconds<nominal*1.10&&maxPosition>840000);
            assert(nonzero-naturalNonzero>10000&&stereoDifference-naturalStereo>100);
            std::printf("PASS actual catalog-backed prologue natural EOF: seconds=%.3f nominal=%.6f maxSource=%u nonzero=%u stereoDifferences=%u; all stream/child/ARAM/parent/DSP resources restored\n",seconds,nominal,maxPosition,nonzero-naturalNonzero,stereoDifference-naturalStereo);
            phase=7;changed.notify_all();
        }
    }
};
Controller* controller=nullptr;
s32 sync(void*) {
    assert(OSGetCurrentThread()!=mainThread);
    std::lock_guard<std::mutex> lock(mutex);controller->tick();return 0;
}
void pcm(s16* samples,u32 frames) {
    assert(OSGetCurrentThread()!=mainThread&&frames==560);
    std::lock_guard<std::mutex> lock(mutex);++controller->dacFrames;
    if(controller->holdOutputExpected)++controller->constantDacFrames;
    for(unsigned i=0;i<frames;++i) {
        if(controller->holdOutputExpected&&(samples[2*i]!=controller->heldLeft||samples[2*i+1]!=controller->heldRight)) {
            std::printf("pause PCM mismatch phase=%u frame=%u sample=%u actual=%d,%d expected=%d,%d\n",controller->phase,controller->frames,i,int(samples[2*i]),int(samples[2*i+1]),int(controller->heldLeft),int(controller->heldRight));
            assert(false&&"Paused output differs from original constant-sample mixer contract");
        }
        controller->nonzero+=samples[2*i]!=0;controller->nonzero+=samples[2*i+1]!=0;
        controller->stereoDifference+=samples[2*i]!=samples[2*i+1];
    }
}
}
int main(int argc,char** argv) {
    if(argc!=2)return 2;
    std::setvbuf(stdout,nullptr,_IONBF,0);mainThread=OSGetCurrentThread();
    const char* path="/AudioRes/Stream/SMG_ev_prolo04_strm.ast";
    assert(nativeBindDvd(std::filesystem::u8path(argv[1])));
    const int entry=DVDConvertPathToEntrynum(path);assert(entry>=0);
    std::vector<u8> stream;assert(nativeReadDvdEntry(entry,stream));JASAramStream::Header header{};
    assert(Native::decodeJasStreamHeader(stream.data(),stream.size(),header)&&header.loop_end==841863&&header.channels==2&&header.loop==0);
    std::vector<u8> packed;const int catalogEntry=DVDConvertPathToEntrynum("/AudioRes/SMR.szs");assert(catalogEntry>=0&&nativeReadDvdEntry(catalogEntry,packed));
    auto archive=decodeYaz0(packed);Catalog catalog;assert(catalog.parse(archive.data(),archive.size())&&catalog.bst&&catalog.bytes==38656&&catalog.bsft==0);
    std::printf("PASS actual SMR.szs ->%zu-byte BAA original dispatch; BST=%u bytes, BSFT commands=%u (real game uses StdSoundInfo fallback)\n",archive.size(),catalog.bytes,catalog.bsft);
    std::vector<u8> storage(0x400000+32);auto base=(reinterpret_cast<std::uintptr_t>(storage.data())+31)&~std::uintptr_t(31);
    JKRExpHeap root(reinterpret_cast<void*>(base),0x400000,nullptr,false);
    JKRHeap::sRootHeap=JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();
    const auto initial=root.getTotalFreeSize();
    auto* dram=JKRSolidHeap::create(0x300000,&root,false);assert(dram);JASKernel::setupRootHeap(dram,0x100000);
    std::vector<u8> backing(0x400000);auto aram=std::make_shared<NativeAramMemory>(backing.data(),backing.size());
    assert(nativeBindAramTransport(aram));
    JAIStream::newMemPool(3);JAISoundChild::newMemPool(160);assert(JASDvd::createThread(16,64,0x4000));JASAramStream::initSystem(header.block_size,2);
    {
        Controller state(catalog.bst,catalog.bytes);controller=&state;
        DSPInit();Native::nativeDspAddressSpace().clear();JASDsp::setDSPMixerLevel(.25f);
        assert(JASDriver::registerDspSyncCallback(sync,nullptr));JASDriver::dacCallbackFunc=pcm;
        std::puts("Starting original audio and DVD workers with retail WBFS stream; no manual DSP stepping or worker fences");
        JASAudioThread::create(16);auto* audio=JASAudioThread::getThreadPointer();assert(audio&&nativeDspAudioRuntimeActive());
        {
            std::unique_lock<std::mutex> lock(mutex);
            const bool complete=changed.wait_for(lock,std::chrono::seconds(50),[&]{return state.phase==7;});
            if(!complete){std::printf("FAIL live retail deadline phase=%u frames=%u position=%u nonzero=%u\n",state.phase,state.frames,state.maxPosition,state.nonzero);std::_Exit(3);}
        }
        audio->stop();const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);
        while(!OSIsThreadTerminated(audio->getThreadRecord())&&std::chrono::steady_clock::now()<deadline)std::this_thread::yield();
        assert(OSIsThreadTerminated(audio->getThreadRecord())&&!nativeDspAudioRuntimeActive());state.drained();
        JASDriver::rejectCallback(sync,nullptr);JASDriver::dacCallbackFunc=nullptr;AIRegisterDMACallback(nullptr);
        audio->~JASAudioThread();JKRHeap::free(audio,dram);controller=nullptr;
    }
    auto* worker=JASAramStream::sLoadThread;worker->~JASTaskThread();JKRHeap::free(worker,dram);
    nativeSetDspAudioBootHooks(nullptr,nullptr);DSPInit();Native::nativeDspAddressSpace().clear();nativeReleaseAramTransport();nativeReleaseDvd();
    assert(root.check()&&JKRThread::sThreadList.getNumLinks()==0);
    std::puts("PASS actual original audio/DVD workers stopped, manager resources reclaimed, native transports unbound and thread list empty");
    assert(!JAISoundInfo::getInstance()&&!JAUSoundInfo::getInstance()&&!JAUSoundTable::getInstance());
    JKRHeap::destroy(dram);
    assert(!JASDram&&!JASAramStream::sReadBuffer&&!JASAramStream::sLoadThread&&!JAIStream::memPool_.getTotalMemCount()&&!JAISoundChild::memPool_.getTotalMemCount()&&root.check()&&root.getTotalFreeSize()==initial);
    std::puts("PASS actual catalog-backed original stream manager, authored volume/stereo children, pause hierarchy and three stop/reuse cycles; child slots and heap reclaimed; normal main return");
}
