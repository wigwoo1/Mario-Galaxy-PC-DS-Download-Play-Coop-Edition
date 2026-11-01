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
static void checkReleased(JASAramStream& stream,StreamEvents& events,WorkerFence& fence){
    fence.wait();assert(events.finished==1&&events.prepared==1);
    for(auto* parent:stream.mChannels)assert(!parent);
    assert(JASChannel::memPool_.getFreeMemCount()==72&&JASDSPChannel::getNumFree()==64);
    assert(!stream.mCallback&&stream._114==1);
    assert(DVDClose(&stream.mDvdFileInfo)); // Fixture owns the opened file lifetime.
}
static void runStream(Harness& audio,int entry,const JASAramStream::Header& header,bool stopEarly){
    StreamEvents events;JASAramStream stream;WorkerFence fence;
    stream.init(0x8000,header.block_size*4*header.channels,StreamEvents::callback,&events);
    assert(stream.prepare(entry,-1));
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(15);
    while(!events.prepared){fence.wait();assert(std::chrono::steady_clock::now()<deadline);}
    assert(events.prepared==1&&!events.finished&&stream.start());
    unsigned subframes=0,nonzero=0;bool requestedStop=false;
    for(;subframes<20000&&!events.finished;++subframes){
        audio.step();fence.wait();
        if(subframes==1)for(unsigned ch=0;ch<stream.mChannelNum;++ch){auto* parent=stream.mChannels[ch];assert(parent);std::printf("stream auto route ch=%u mode=%u id=%04x\n",ch,parent->mDspCh->mChannel->_58,parent->mDspCh->mChannel->_10[0][0]);assert(parent->mDspCh->mChannel->_58==1&&parent->mDspCh->mChannel->_10[0][0]==0);}
        for(auto sample:audio.output)nonzero+=sample!=0;
        if(stopEarly&&subframes==30){assert(stream.stop(10));requestedStop=true;}
        if(subframes%4000==0)std::printf("stream trace subframe=%u block=%u position=%u pending=%u parentFree=%u dspUse=%u\n",subframes,stream.mBlock,u32(stream._0B8),u32(stream._118),JASChannel::memPool_.getFreeMemCount(),JASDSPChannel::getNumUse());
    }
    std::printf("stream %s terminal subframes=%u nonzero=%u prepared=%u finished=%u\n",stopEarly?"stop":"natural",subframes,nonzero,events.prepared.load(),events.finished.load());
    assert(nonzero>0&&events.finished==1&&requestedStop==stopEarly);
    if(!stopEarly)assert(subframes>9000); // Whole retail841863-sample file, not one block.
    else assert(subframes<200);
    checkReleased(stream,events,fence);
    std::puts(stopEarly?"PASS retail stream explicit release reaches original parent destruction, finishTask and pool/DSP return":"PASS whole retail stream natural renderer EOF reaches original parent destruction, finishTask and pool/DSP return");
}
static void busBounds(){
    JASDsp::TChannel voice{};voice.init();
    const u16 addresses[12]={0,0xD00,0xD60,0xDC0,0xE20,0xE80,0xEE0,0xCA0,0xF40,0xFA0,0xB00,0x9A0};
    for(unsigned slot=0;slot<6;++slot)for(unsigned bus=0;bus<12;++bus){voice.setBusConnect(slot,bus);assert(voice._10[slot][0]==addresses[bus]);}
    for(auto bus:{12,254,255}){bool rejected=false;try{voice.setBusConnect(0,bus);}catch(const std::out_of_range&){rejected=true;}assert(rejected);}
    bool rejected=false;try{voice.setBusConnect(6,0);}catch(const std::out_of_range&){rejected=true;}assert(rejected);
    voice.initAutoMixer();voice.setBusConnect(0,255);assert(voice._58==1&&voice._10[0][0]==0);
    std::puts("PASS all72 ordinary bus routes, invalid bus/slot rejection and original automatic-mixer255 sentinel without table overread");
}
int main(int argc,char**argv){
    if(argc!=2)return 2;std::setvbuf(stdout,nullptr,_IONBF,0);busBounds();
    const char* track="/AudioRes/Stream/SMG_ev_prolo04_strm.ast";
    assert(nativeBindDvd(std::filesystem::u8path(argv[1])));int entry=DVDConvertPathToEntrynum(track);assert(entry>=0);
    std::vector<u8> bytes;assert(nativeReadDvdEntry(entry,bytes));JASAramStream::Header header{};
    assert(Native::decodeJasStreamHeader(bytes.data(),bytes.size(),header));
    assert(header.format==1&&header.channels==2&&!header.loop&&header.loop_end==841863);
    std::printf("retail %s bytes=%zu samples=%d rate=%d blockBytes=%u\n",track,bytes.size(),header.loop_end,header._10,header.block_size);
    std::vector<u8> storage(0x400000+32);auto base=(reinterpret_cast<std::uintptr_t>(storage.data())+31)&~std::uintptr_t(31);
    JKRExpHeap root(reinterpret_cast<void*>(base),0x400000,nullptr,false);
    JKRHeap::sRootHeap=JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();
    auto* dram=JKRSolidHeap::create(0x300000,&root,false);assert(dram);JASKernel::setupRootHeap(dram,0x100000);
    JASChannel::newMemPool(0x48);JASChannel::initBankDisposeMsgQueue();assert(JASDvd::createThread(16,64,0x4000));
    JASAramStream::initSystem(header.block_size,2);
    {Harness audio;runStream(audio,entry,header,false);runStream(audio,entry,header,true);}
    // Fence already drained all actual callbacks; cancel/join the real worker.
    auto* worker=JASAramStream::sLoadThread;worker->~JASTaskThread();JKRHeap::free(worker,dram);JASAramStream::sLoadThread=nullptr;
    nativeReleaseDvd();assert(root.check()&&JKRThread::sThreadList.getNumLinks()==0);
    std::puts("PASS real DVD task worker stopped and host runtime/ARAM/DVD unbound. Solid-heap individual frees report warnings as designed; whole pool/heap teardown is not tested. No JAI manager, real-time scheduling, device or game verified.");
    std::_Exit(0);
}
