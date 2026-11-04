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
    unsigned nonzero=0,stereoDifference=0,stopFrame=0;
    bool played=false;
    std::chrono::steady_clock::time_point naturalStart;
    double naturalSeconds=0;
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
    void tick() {
        if(++subframes%7 || phase==5)return;
        ++frames;
        if(phase==0){start();naturalStart=std::chrono::steady_clock::now();phase=1;}
        if(handle.isSoundAttached()&&phase==1) {
            auto* stream=handle.getSound()->asStream();
            maxPosition=std::max(maxPosition,unsigned(stream->inner_.aramStream._0B8));
            played|=stream->mPrepareState==4;
        }
        manager.calc();manager.mixOut();
        if(phase==1&&!manager.isActive()) {
            drained();assert(played&&maxPosition>840000&&nonzero>10000&&stereoDifference>100);
            naturalSeconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-naturalStart).count();
            assert(naturalSeconds>=20&&naturalSeconds<55);
            std::printf("PASS authentic841863-sample/channel retail stream natural EOF on original live thread: seconds=%.3f frames=%u maxPosition=%u nonzero=%u stereoDifference=%u; all pools/ARAM reclaimed\n",naturalSeconds,frames,maxPosition,nonzero,stereoDifference);
            start();phase=2;
        } else if(phase==2&&handle.isSoundAttached()) {
            auto* stream=handle.getSound()->asStream();
            if(stream->mPrepareState==4&&u32(stream->inner_.aramStream._0B8)>4000) {
                handle->stop(10);stopFrame=frames;phase=3;
            }
        } else if(phase==3&&!manager.isActive()) {
            drained();assert(frames>stopFrame);
            std::printf("PASS idle DVD metadata and freed stream/ARAM objects reused during live playback; normal stop(10) drains after%u manager frames\n",frames-stopFrame);
            start();handle->stop();phase=4;
        } else if(phase==4&&!manager.isActive()) {
            drained();phase=5;
            std::puts("PASS live manager cancellation before allocation, original3 stream slots/2 ARAM buffers/72 parents/64 DSP slots preserved");
            changed.notify_all();
        }
        if(frames%300==0)std::printf("retail live trace phase=%u managerFrames=%u sourcePosition=%u dac=%u nonzero=%u\n",phase,frames,maxPosition,dacFrames,nonzero);
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
    for(unsigned i=0;i<frames;++i) {
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
    std::vector<u8> storage(0x400000+32);auto base=(reinterpret_cast<std::uintptr_t>(storage.data())+31)&~std::uintptr_t(31);
    JKRExpHeap root(reinterpret_cast<void*>(base),0x400000,nullptr,false);
    JKRHeap::sRootHeap=JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();
    auto* dram=JKRSolidHeap::create(0x300000,&root,false);assert(dram);JASKernel::setupRootHeap(dram,0x100000);
    std::vector<u8> backing(0x400000);auto aram=std::make_shared<NativeAramMemory>(backing.data(),backing.size());
    assert(nativeBindAramTransport(aram));
    JAIStream::newMemPool(3);assert(JASDvd::createThread(16,64,0x4000));JASAramStream::initSystem(header.block_size,2);
    {
        Controller state(path);controller=&state;
        DSPInit();Native::nativeDspAddressSpace().clear();JASDsp::setDSPMixerLevel(.25f);
        assert(JASDriver::registerDspSyncCallback(sync,nullptr));JASDriver::dacCallbackFunc=pcm;
        std::puts("Starting original audio and DVD workers with retail WBFS stream; no manual DSP stepping or worker fences");
        JASAudioThread::create(16);auto* audio=JASAudioThread::getThreadPointer();assert(audio&&nativeDspAudioRuntimeActive());
        {
            std::unique_lock<std::mutex> lock(mutex);
            const bool complete=changed.wait_for(lock,std::chrono::seconds(55),[&]{return state.phase==5;});
            if(!complete){std::printf("FAIL live retail deadline phase=%u frames=%u position=%u nonzero=%u\n",state.phase,state.frames,state.maxPosition,state.nonzero);std::_Exit(3);}
        }
        audio->stop();const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);
        while(!OSIsThreadTerminated(audio->getThreadRecord())&&std::chrono::steady_clock::now()<deadline)std::this_thread::yield();
        assert(OSIsThreadTerminated(audio->getThreadRecord())&&!nativeDspAudioRuntimeActive());state.drained();
        JASDriver::rejectCallback(sync,nullptr);JASDriver::dacCallbackFunc=nullptr;AIRegisterDMACallback(nullptr);
        audio->~JASAudioThread();JKRHeap::free(audio,dram);controller=nullptr;
    }
    auto* worker=JASAramStream::sLoadThread;worker->~JASTaskThread();JKRHeap::free(worker,dram);JASAramStream::sLoadThread=nullptr;
    nativeSetDspAudioBootHooks(nullptr,nullptr);DSPInit();Native::nativeDspAddressSpace().clear();nativeReleaseAramTransport();nativeReleaseDvd();
    assert(root.check()&&JKRThread::sThreadList.getNumLinks()==0);
    std::puts("PASS actual original audio/DVD workers stopped, manager resources reclaimed, native transports unbound and thread list empty");
    std::puts("No game execution/full spatial sound-info/audible endpoint/fidelity/race-free memory-model/static-pool shutdown claim; process-lifetime pool backing retained");
    std::_Exit(0);
}
