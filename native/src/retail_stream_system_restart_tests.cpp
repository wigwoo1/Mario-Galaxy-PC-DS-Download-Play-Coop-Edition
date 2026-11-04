#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <array>
#include <Native/JasStreamSystemLifetime.hpp>
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
struct Controller {
    JAUStreamDataMgr_StreamFileTable table;
    JASHeap root; // Must outlive the static-ARAM manager's child heaps.
    JAUStreamStaticAramMgr aram;
    JAIStreamMgr manager{false};
    JAISoundHandle handle;
    std::vector<u8> bsft;
    unsigned phase=0,subframes=0,frames=0,dacFrames=0,maxPosition=0;
    unsigned nonzero=0,stereoDifference=0,stopFrame=0,pauseFrame=0;
    unsigned heldPosition=0,resumeNonzero=0,startNonzero=0,cycles=0,constantDacFrames=0;
    bool holdOutputExpected=false;
    s16 heldLeft=0,heldRight=0;
    explicit Controller(const char* path):bsft(12+std::strlen(path)+1) {
        std::memcpy(bsft.data(),"bsft",4);bsft[7]=1;bsft[11]=12;
        std::memcpy(bsft.data()+12,path,std::strlen(path)+1);table.init(bsft.data());
        root.initRootHeap(reinterpret_cast<void*>(0x8000),0x300000);aram.alloc(36,&root);
        assert(aram.mSize==2);manager.setStreamDataMgr(&table);manager.setStreamAramMgr(&aram);
    }
    void start() {
        assert(manager.startSound(JAISoundID(0x02000000),&handle,nullptr));
        assert(handle.isSoundAttached()&&JAIStream::memPool_.getFreeMemCount()==2);
    }
    void drained() {
        assert(!manager.isActive()&&!handle.isSoundAttached()&&aram.mBits.none());
        assert(JAIStream::memPool_.getFreeMemCount()==3&&JASChannel::memPool_.getFreeMemCount()==72&&JASDSPChannel::getNumFree()==64);
    }
    void held(JAIStream* stream) {
        assert(stream&&stream->mPrepareState==4&&stream->mIsPaused);
        assert(u32(stream->inner_.aramStream._0B8)==heldPosition);
        assert(stream->inner_.aramStream._0AE&1);
        assert(aram.mBits.count()==1&&JAIStream::memPool_.getFreeMemCount()==2);
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
                if(++cycles==3){phase=7;assert(constantDacFrames>=180);changed.notify_all();}
                else {manager.pause(false);phase=0;}
            }
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
unsigned streamCallbackCount() {
    unsigned count=0;
    for(const auto& entry:JASDriver::sSubFrameCallback.mCallbacks)if(entry.mCallback==JASAramStream::dvdErrorCheck&&entry.mArg==nullptr)++count;
    return count;
}
unsigned allSubframeCallbacks() {
    unsigned count=0;for(const auto& entry:JASDriver::sSubFrameCallback.mCallbacks)count+=entry.mCallback!=nullptr;return count;
}
s32 neverRun(void*) { assert(false&&"Initialization-only callback unexpectedly executed");return 0; }
void initializationFailures(u32 block) {
    for(unsigned mode=0;mode<4;++mode) {
        auto* memory=VirtualAlloc(nullptr,0x100000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);assert(memory);
        {
            JKRExpHeap root(memory,0x100000,nullptr,false);
            JKRHeap::sRootHeap=JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();
            const auto initial=root.getTotalFreeSize();
            auto* heap=JKRSolidHeap::create(0x40000,&root,false);assert(heap);JASDram=heap;
            assert(!Native::jasStreamSystemOwner()&&!allSubframeCallbacks());
            u32 bytes=0;assert(Native::jasStreamReadBufferSize(block,2,bytes));
            if(mode<2) {
                if(!mode)assert(heap->alloc(heap->getFreeSize(),4));
                else {
                    const u32 padding=u32((-reinterpret_cast<std::uintptr_t>(heap->mSolidHead))&31);
                    const u32 required=bytes+padding;
                    assert(heap->alloc(heap->getFreeSize()-required,-8));assert(heap->getFreeSize()==required);
                }
                bool failed=false;try{JASAramStream::initSystem(block,2);}catch(const std::bad_alloc&){failed=true;}
                assert(failed&&!streamCallbackCount()&&!Native::jasStreamSystemOwner()&&!JASAramStream::sReadBuffer&&!JASAramStream::sLoadThread&&!JASAramStream::sBlockSize&&!JASAramStream::sChannelMax);
            } else if(mode==2) {
                for(unsigned i=0;i<JASCALLBACK_MAX_CALLBACKS;++i)assert(JASDriver::registerSubFrameCallback(neverRun,reinterpret_cast<void*>(std::uintptr_t(i+1))));
                const auto freeBefore=heap->getFreeSize();JASAramStream::initSystem(block,2);
                assert(!Native::jasStreamSystemOwner()&&!JASAramStream::sReadBuffer&&heap->getFreeSize()==freeBefore&&allSubframeCallbacks()==JASCALLBACK_MAX_CALLBACKS);
                for(unsigned i=0;i<JASCALLBACK_MAX_CALLBACKS;++i)assert(JASDriver::rejectCallback(neverRun,reinterpret_cast<void*>(std::uintptr_t(i+1)))==1);
            } else {
                assert(JASDriver::registerSubFrameCallback(JASAramStream::dvdErrorCheck,nullptr));
                bool rejected=false;try{JASAramStream::initSystem(block,2);}catch(const std::logic_error&){rejected=true;}
                assert(rejected&&streamCallbackCount()==1&&!Native::jasStreamSystemOwner()&&!JASAramStream::sReadBuffer);
                assert(JASDriver::rejectCallback(JASAramStream::dvdErrorCheck,nullptr)==1);
            }
            assert(!allSubframeCallbacks());JKRHeap::destroy(heap);JASDram=nullptr;
            assert(root.check()&&root.getTotalFreeSize()==initial);
        }
        assert(VirtualFree(memory,0,MEM_RELEASE));
    }
    std::puts("PASS actual read-buffer/owner OOM rollback, full callback-table no-allocation path and preserved foreign registration; parent heaps reclaimed");
}
int main(int argc,char** argv) {
    if(argc!=2&&argc!=3)return 2;
    std::setvbuf(stdout,nullptr,_IONBF,0);mainThread=OSGetCurrentThread();
    const char* path="/AudioRes/Stream/SMG_ev_prolo04_strm.ast";
    assert(nativeBindDvd(std::filesystem::u8path(argv[1])));
    const int entry=DVDConvertPathToEntrynum(path);assert(entry>=0);
    std::vector<u8> stream;assert(nativeReadDvdEntry(entry,stream));JASAramStream::Header header{};
    assert(Native::decodeJasStreamHeader(stream.data(),stream.size(),header)&&header.loop_end==841863&&header.channels==2&&header.loop==0);
    initializationFailures(header.block_size);
    std::array<void*,3> reservations{};
    for(unsigned generation=0;generation<3;++generation) {
    if(generation)assert(nativeBindDvd(std::filesystem::u8path(argv[1])));
    auto* memory=VirtualAlloc(nullptr,0x400000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);assert(memory);reservations[generation]=memory;
    {
    const auto base=reinterpret_cast<std::uintptr_t>(memory);
    JKRExpHeap root(reinterpret_cast<void*>(base),0x400000,nullptr,false);
    JKRHeap::sRootHeap=JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();
    const auto initial=root.getTotalFreeSize();
    auto* dram=JKRSolidHeap::create(0x300000,&root,false);assert(dram);JASKernel::setupRootHeap(dram,0x100000);
    std::vector<u8> backing(0x400000);auto aram=std::make_shared<NativeAramMemory>(backing.data(),backing.size());
    assert(nativeBindAramTransport(aram));
    JAIStream::newMemPool(3);assert(JASDvd::createThread(16,64,0x4000));JASAramStream::initSystem(header.block_size,2);
    assert(streamCallbackCount()==1&&JKRHeap::findFromRoot(JASAramStream::sReadBuffer)==dram);
    if(argc==3) {
        auto* first=JASAramStream::sReadBuffer;JASAramStream::initSystem(header.block_size,2);
        std::printf("Repeated init callbacks=%u oldReadBuffer=%p currentReadBuffer=%p\n",streamCallbackCount(),first,JASAramStream::sReadBuffer);
        assert(streamCallbackCount()==1&&first==JASAramStream::sReadBuffer&&"Repeated initialization duplicates stream callback/storage");
        bool rejected=false;try{JASAramStream::initSystem(header.block_size+32,2);}catch(const std::logic_error&){rejected=true;}
        assert(rejected&&streamCallbackCount()==1&&first==JASAramStream::sReadBuffer);
    }
    {
        Controller state(path);controller=&state;
        Native::nativeDspAddressSpace().clear();JASDsp::setDSPMixerLevel(.25f);
        assert(JASDriver::registerDspSyncCallback(sync,nullptr));JASDriver::dacCallbackFunc=pcm;
        std::puts("Starting original audio and DVD workers with retail WBFS stream; no manual DSP stepping or worker fences");
        JASAudioThread::create(16);auto* audio=JASAudioThread::getThreadPointer();assert(audio&&nativeDspAudioRuntimeActive());
        {
            std::unique_lock<std::mutex> lock(mutex);
            const bool complete=changed.wait_for(lock,std::chrono::seconds(20),[&]{return state.phase==7;});
            if(!complete){std::printf("FAIL live retail deadline phase=%u frames=%u position=%u nonzero=%u\n",state.phase,state.frames,state.maxPosition,state.nonzero);std::_Exit(3);}
        }
        audio->stop();const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);
        while(!OSIsThreadTerminated(audio->getThreadRecord())&&std::chrono::steady_clock::now()<deadline)std::this_thread::yield();
        assert(OSIsThreadTerminated(audio->getThreadRecord())&&!nativeDspAudioRuntimeActive());state.drained();
        JASDriver::rejectCallback(sync,nullptr);JASDriver::dacCallbackFunc=nullptr;AIRegisterDMACallback(nullptr);
        audio->~JASAudioThread();JKRHeap::free(audio,dram);controller=nullptr;
    }
    auto* worker=JASAramStream::sLoadThread;worker->~JASTaskThread();JKRHeap::free(worker,dram);
    Native::nativeDspAddressSpace().clear();nativeReleaseAramTransport();nativeReleaseDvd();
    assert(root.check()&&JKRThread::sThreadList.getNumLinks()==0);
    std::puts("PASS actual original audio/DVD workers stopped, manager resources reclaimed, native transports unbound and thread list empty");
    UNK_BOOL_A=UNK_BOOL_B=true; // Workers stopped: verify heap disposal clears both error flags too.
    JKRHeap::destroy(dram);
    std::printf("Retail generation%u heap disposed: callbacks=%u loadThread=%p readBuffer=%p blockSize=%u channelMax=%u\n",generation+1,streamCallbackCount(),JASAramStream::sLoadThread,JASAramStream::sReadBuffer,JASAramStream::sBlockSize,JASAramStream::sChannelMax);
    assert(streamCallbackCount()==0&&!JASAramStream::sLoadThread&&!JASAramStream::sReadBuffer&&!JASAramStream::sBlockSize&&!JASAramStream::sChannelMax&&"Released stream system leaves callback/static-buffer references");
    assert(!Native::jasStreamSystemOwner()&&!UNK_BOOL_A&&!UNK_BOOL_B&&!JASDvd::getThreadPointer());
    assert(!JASDram&&JAIStream::memPool_.getTotalMemCount()==0&&JASChannel::memPool_.getTotalMemCount()==0&&root.check()&&root.getTotalFreeSize()==initial);
    }
    assert(VirtualFree(memory,0x400000,MEM_DECOMMIT));
    std::printf("PASS retail stream system generation%u: old physical heap backing decommitted\n",generation+1);
    }
    for(auto* reservation:reservations)assert(VirtualFree(reservation,0,MEM_RELEASE));
    nativeSetDspAudioBootHooks(nullptr,nullptr);DSPInit();
    std::puts("PASS three authentic JAI stream/audio/DVD system lifetimes, nine pause/resume/stop cycles, all callbacks/stream statics reclaimed and normal main return");
}
