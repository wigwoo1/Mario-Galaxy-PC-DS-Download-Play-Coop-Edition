#include <Native/HeapPlacement.hpp>
#include <Native/DspAddressSpace.hpp>
#include <Native/DspAudioRuntime.hpp>
#include <Native/DspMailbox.hpp>
#include <JSystem/JAudio2/JASDSPChannel.hpp>
#include <JSystem/JAudio2/JASWaveInfo.hpp>
#include <JSystem/JAudio2/JASHeapCtrl.hpp>
#include <JSystem/JAudio2/dsptask.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <revolution/dsp.h>
#include "native_jkr_aram_port.hpp"
#include "dsp_audio_dispatch_test_callback.hpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <vector>
TestAudioThread* TestAudioThread::current=nullptr;


#include <JSystem/JAudio2/JASChannel.hpp>
#include <JSystem/JAudio2/JASDriverIF.hpp>
#include <cstdlib>

#include <JSystem/JAudio2/JASAramStream.hpp>
#include <JSystem/JAudio2/JASDvdThread.hpp>
#include <Native/JasStreamEndian.hpp>
#include <JSystem/JAudio2/JASCriticalSection.hpp>
#include <chrono>
#include <thread>
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include "native_dvd_port.hpp"
struct Harness {
    TestAudioThread audio;
    std::vector<u8> backing=std::vector<u8>(0x400000);
    std::shared_ptr<NativeAramMemory> aram=std::make_shared<NativeAramMemory>(backing.data(),backing.size());
    std::array<s16,160> output{};
    u32 left=0,right=0;
    Harness(){
        DSPInit();Native::nativeDspAddressSpace().clear();assert(nativeBindAramTransport(aram));
        std::array<u8,416> source{};for(unsigned i=0;i<200;++i){source[i*2]=0x27;source[i*2+1]=0x10;}
        assert(nativeUploadAram(0x4000,source.data(),source.size()));
        nativeConfigureDspAudioRuntime();JASDsp::boot(TestAudioThread::DSPCallback);
        assert(nativeDspAudioRuntimeActive());
        JASDsp::initBuffer();JASDSPChannel::initAll();JASDsp::setDSPMixerLevel(.25f);
        auto& space=Native::nativeDspAddressSpace();left=space.map(output.data(),sizeof(output));right=space.addressOf(output.data()+80,160);
        assert(left&&right&&JASDSPChannel::getNumFree()==64);
    }
    ~Harness(){
        nativeStopDspAudioRuntime();nativeSetDspAudioBootHooks(nullptr,nullptr);DSPInit();
        Native::nativeDspAddressSpace().clear();nativeReleaseAramTransport();
        // Unregister runtime/mappings before the process-lifetime heap exits.
        JASDSPChannel::sDspChannels=nullptr;JASDsp::CH_BUF=nullptr;JASDsp::FX_BUF=nullptr;
    }
    void step(){
        output.fill(s16(0x5A5A));
        JASDsp::syncFrame(1,left,right);
        JASDSPChannel::updateAll(); // Real updateProc and all4 original releaseHalt groups.
        JASDriver::subframeCallback();
        nativeDspServiceHostAudio();
        OSMessage message=nullptr;assert(OSReceiveMessage(&audio.queue,&message,OS_MESSAGE_NOBLOCK));
        assert(std::intptr_t(message)==AUDIOMSG_DSP&&!OSReceiveMessage(&audio.queue,&message,OS_MESSAGE_NOBLOCK));
        assert(!nativeDspAudioRuntimeFailed());
    }
};
struct WorkerFence {
    std::mutex mutex;std::condition_variable changed;bool done=false;
    static void callback(void* context){auto& f=*static_cast<WorkerFence*>(context);std::lock_guard<std::mutex> lock(f.mutex);f.done=true;f.changed.notify_one();}
    void wait(){
        std::unique_lock<std::mutex> lock(mutex);done=false;
        assert(JASAramStream::sLoadThread->sendCmdMsg(callback,this));
        assert(changed.wait_for(lock,std::chrono::seconds(10),[this]{return done;}));
    }
};
struct StreamEvents {
    std::atomic<unsigned> prepared{0},finished{0};
    static void callback(u32 event,JASAramStream*,void* context){
        auto& events=*static_cast<StreamEvents*>(context);
        // Original names are reversed relative to preparation/completion meaning.
        if(event==JASAramStream::CB_STOP)++events.prepared;
        else if(event==JASAramStream::CB_START)++events.finished;
        else assert(false);
    }
};

#include <JSystem/JAudio2/JAIStreamMgr.hpp>
#include <JSystem/JAudio2/JAUStreamDataMgr.hpp>
#include <JSystem/JAudio2/JAUStreamFileTable.hpp>
#include <cstring>
static std::vector<u8> streamTable(const char* path){
    std::vector<u8> data(12+std::strlen(path)+1);std::memcpy(data.data(),"bsft",4);data[7]=1;data[11]=12;
    std::memcpy(data.data()+12,path,std::strlen(path)+1);return data;
}
struct ManagerHarness {
    Harness& audio;WorkerFence fence;JAUStreamDataMgr_StreamFileTable table;JASHeap root;
    JAUStreamStaticAramMgr aram;JAIStreamMgr manager{false};std::vector<u8> tableBytes;unsigned nonzero=0;
    ManagerHarness(Harness& a,const char* path):audio(a),tableBytes(streamTable(path)){
        root.initRootHeap(reinterpret_cast<void*>(0x8000),0x300000);
        aram.alloc(36,&root);assert(aram.mSize==2);table.init(tableBytes.data());
        manager.setStreamDataMgr(&table);manager.setStreamAramMgr(&aram);
        assert(JAIStream::memPool_.getTotalMemCount()==3&&JAIStream::memPool_.getFreeMemCount()==3);
    }
    void frame(){
        manager.calc();manager.mixOut();
        for(unsigned s=0;s<7;++s){audio.step();fence.wait();for(auto sample:audio.output)nonzero+=sample!=0;}
    }
    void start(JAISoundHandle& handle){
        const bool result=manager.startSound(JAISoundID(0x02000000),&handle,nullptr);
        std::printf("startSound result=%u attached=%u free=%u active=%d\n",result,handle.isSoundAttached(),JAIStream::memPool_.getFreeMemCount(),manager.getNumActiveStreams());
        assert(result&&handle.isSoundAttached());
    }
    void drained(){
        assert(!manager.isActive()&&aram.mBits.none());
        assert(JAIStream::memPool_.getFreeMemCount()==3&&JASChannel::memPool_.getFreeMemCount()==72&&JASDSPChannel::getNumFree()==64);
    }
    void waitEmpty(unsigned limit=100){unsigned frames=0;while(manager.isActive()&&frames++<limit)frame();drained();}
};
static void capacityReuse(Harness& audio,const char* path){
    ManagerHarness h(audio,path);std::array<JAISoundHandle,4> handles;
    assert(!h.manager.startSound(JAISoundID(0x02000001),&handles[3],nullptr)&&!handles[3].isSoundAttached()&&!h.manager.isActive()&&JAIStream::memPool_.getFreeMemCount()==3);
    for(unsigned i=0;i<3;++i)h.start(handles[i]);
    assert(!h.manager.startSound(JAISoundID(0x02000000),&handles[3],nullptr)&&!handles[3].isSoundAttached());
    for(unsigned i=0;i<20;++i)h.frame();
    assert(h.aram.mBits.count()==2&&JASDSPChannel::getNumUse()==4);
    auto* waiting=handles[2].getSound()->asStream();assert(waiting->mPrepareState==0);
    handles[0]->stop();unsigned frames=0;
    while(waiting->mPrepareState!=4&&frames++<100)h.frame();
    assert(!handles[0].isSoundAttached()&&waiting->mPrepareState==4&&h.manager.getNumActiveStreams()==2&&JAIStream::memPool_.getFreeMemCount()==1);
    h.start(handles[3]);h.manager.stop();h.waitEmpty();
    for(auto& handle:handles)assert(!handle.isSoundAttached());
    for(unsigned cycle=0;cycle<10;++cycle){
        for(unsigned i=0;i<3;++i)h.start(handles[i]);
        for(unsigned i=0;i<10;++i)h.frame();
        assert(JASDSPChannel::getNumUse()==4&&handles[0].getSound()->asStream()->mPrepareState==4&&handles[1].getSound()->asStream()->mPrepareState==4);
        h.manager.stop();h.waitEmpty();
    }
    assert(h.nonzero>0);
    std::puts("PASS original3-object/2-ARAM-buffer capacity, fourth rejection, waiting-stream handoff and10 full reuse/stop cycles: lists, handles, stream/parent/DSP pools restored");
}
static void naturalEnd(Harness& audio,const char* path){
    ManagerHarness h(audio,path);JAISoundHandle handle;h.start(handle);unsigned frames=0;
    while(h.manager.isActive()&&frames++<1800){h.frame();if(frames%300==0&&handle.isSoundAttached()){
        auto* sound=handle.getSound()->asStream();auto& stream=sound->inner_.aramStream;auto* parent=stream.mChannels[0];auto* voice=parent&&parent->mDspCh?parent->mDspCh->mChannel:nullptr;
        std::printf("natural trace frame=%u state=%u prepare=%d stopped=%u finished=%u position=%u block=%u ring=%u flags=%u remaining=%u pause=%u eof=%u pending=%u\n",frames,unsigned(sound->mStatus.getState()),sound->mPrepareState,unsigned(sound->mIsStreamStopped),unsigned(sound->mIsStreamStarted),u32(stream._0B8),stream.mBlock,stream.mBufCount,stream._0AE,voice?voice->_74:0,voice?voice->mPauseFlag:0,voice?voice->mIsFinished:0,u32(stream._118));
    }}
    h.drained();assert(frames>1400&&!handle.isSoundAttached()&&h.nonzero>0);
    std::printf("PASS whole retail natural end through actual JAI callbacks/calc/freeDeadStream: frames=%u nonzero=%u, handle detached and3 stream slots/2 ARAM buffers reclaimed\n",frames,h.nonzero);
}
static void preparationCancel(Harness& audio,const char* path){
    ManagerHarness h(audio,path);JAISoundHandle handle;
    h.start(handle);handle->stop();h.waitEmpty();
    h.start(handle);h.manager.mixOut();assert(handle.getSound()->asStream()->mPrepareState==1);
    handle->stop();h.waitEmpty();
    h.start(handle);assert(handle->mStatus.lockWhenPrepared());
    unsigned frames=0;while(handle->mStatus.getState()!=JAISoundStatus_::State_LOCK_READY&&frames++<100)h.frame();
    assert(handle.getSound()->asStream()->mPrepareState==3&&handle->mStatus.getState()==JAISoundStatus_::State_LOCK_READY&&JASDSPChannel::getNumUse()==0);
    handle->stop();h.waitEmpty();assert(!handle.isSoundAttached());
    std::puts("PASS actual JAI cancellation before allocation, after ARAM allocation and prepared/locked before playback: no stale callbacks, list entries or held stream/ARAM slots");
}
int main(int argc,char**argv){
    if(argc!=2)return 2;std::setvbuf(stdout,nullptr,_IONBF,0);
    const char* track="/AudioRes/Stream/SMG_ev_prolo04_strm.ast";
    assert(nativeBindDvd(std::filesystem::u8path(argv[1])));int entry=DVDConvertPathToEntrynum(track);assert(entry>=0);
    std::vector<u8> bytes;assert(nativeReadDvdEntry(entry,bytes));JASAramStream::Header header{};
    assert(Native::decodeJasStreamHeader(bytes.data(),bytes.size(),header)&&header.loop_end==841863);
    std::vector<u8> storage(0x400000+32);auto base=(reinterpret_cast<std::uintptr_t>(storage.data())+31)&~std::uintptr_t(31);
    JKRExpHeap root(reinterpret_cast<void*>(base),0x400000,nullptr,false);
    JKRHeap::sRootHeap=JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();
    auto* dram=JKRSolidHeap::create(0x300000,&root,false);assert(dram);JASKernel::setupRootHeap(dram,0x100000);
    JASChannel::newMemPool(72);JAIStream::newMemPool(3);JASChannel::initBankDisposeMsgQueue();assert(JASDvd::createThread(16,64,0x4000));
    JASAramStream::initSystem(header.block_size,2);
    {Harness audio;capacityReuse(audio,track);naturalEnd(audio,track);preparationCancel(audio,track);}
    auto* worker=JASAramStream::sLoadThread;worker->~JASTaskThread();JKRHeap::free(worker,dram);JASAramStream::sLoadThread=nullptr;
    nativeReleaseDvd();assert(root.check()&&JKRThread::sThreadList.getNumLinks()==0);
    std::puts("PASS worker/runtime cleanup; process-lifetime pool backing retained. No real-time/full audio thread/device/game verification.");std::_Exit(0);
}
