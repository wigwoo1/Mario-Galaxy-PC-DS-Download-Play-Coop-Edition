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
struct ParentSound {
    unsigned starts=0,plays=0,stops=0;
    JASChannel* owner=nullptr;
    static void callback(u32 event,JASChannel* channel,JASDsp::TChannel*,void* context){
        auto& sound=*static_cast<ParentSound*>(context);
        assert(sound.owner==channel);
        if(event==JASChannel::CB_START)++sound.starts;
        else if(event==JASChannel::CB_PLAY)++sound.plays;
        else if(event==JASChannel::CB_STOP){++sound.stops;sound.owner=nullptr;}
        else assert(event==JASChannel::CB_TIMER);
        // Real JASChannel owns its DSP handle and deletion; no fixture free().
    }
    JASChannel* create(float pan=.5f,bool loop=false){
        assert(!owner);owner=new JASChannel(callback,this);assert(owner);
        owner->mWaveInfo.mFormat=3;owner->mWaveInfo.mLoopFlags=loop;
        owner->mWaveInfo.mSampleCount=200;owner->mWaveInfo.mSampleLoopStart=40;
        owner->mWaveInfo.mSampleLoopEnd=200;owner->mWaveInfo.mpLastSample=owner->mWaveInfo.mpPenultSample=0;
        owner->mWavePtr=0x4000;owner->setInitPan(pan);return owner;
    }
};
struct Harness {
    TestAudioThread audio;
    std::array<u8,0x10000> backing{};
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
        nativeDspServiceHostAudio();
        OSMessage message=nullptr;assert(OSReceiveMessage(&audio.queue,&message,OS_MESSAGE_NOBLOCK));
        assert(std::intptr_t(message)==AUDIOMSG_DSP&&!OSReceiveMessage(&audio.queue,&message,OS_MESSAGE_NOBLOCK));
        assert(!nativeDspAudioRuntimeFailed());
    }
};
static auto& pool(){return JASChannel::memPool_;}
static void layout(){
    static_assert(sizeof(JASChannel::MixConfig)==2);
    for(unsigned word=0;word<65536;++word){
        JASChannel::MixConfig config;config.whole=word;
        assert(config.upper==(word>>8)&&config.lower0==((word>>4)&15)&&config.lower1==(word&15));
        config.upper=(word>>8)^255;config.lower0=((word>>4)&15)^15;config.lower1=(word&15)^15;
        assert(config.whole==(word^65535));
    }
    std::puts("PASS all65536 packed mixer words: bus and two modifiers decode/write back with original semantics and size2");
}
static void naturalParents(Harness& audio){
    for(unsigned cycle=0;cycle<10;++cycle){
        ParentSound sound;auto* parent=sound.create(float(cycle%2));
        assert(pool().getFreeMemCount()==71&&parent->play());auto* dsp=parent->mDspCh;
        audio.step();assert(sound.starts==1&&sound.stops==0);
        assert(dsp->mChannel->_10[0][0]==0xD00&&dsp->mChannel->_10[1][0]==0xD60);
        for(unsigned i=4;i<80;++i){
            const auto audible=audio.output[(cycle%2)*80+i],silent=audio.output[(1-cycle%2)*80+i];
            assert(audible>1500&&audible<2000&&silent==0);
        }
        audio.step();audio.step();assert(sound.owner&&sound.stops==0&&dsp->mChannel->isFinish());
        audio.step();assert(!sound.owner&&sound.stops==1&&pool().getFreeMemCount()==72);
        assert(JASDSPChannel::getNumFree()==64&&JASDSPChannel::getNumUse()==0);
    }
    std::puts("PASS original parent start/mixer hard-left/right + natural EOF/destructor:10 sounds return their own pool allocation and DSP slot without fixture deletion");
}
static void pauseAndDispose(Harness& audio){
    ParentSound paused;auto* parent=paused.create();parent->setPauseFlag(true);assert(parent->play());
    audio.step();const auto remaining=parent->mDspCh->mChannel->_74;
    for(unsigned frame=0;frame<5;++frame){audio.step();assert(parent->mDspCh->mChannel->_74==remaining&&paused.stops==0);for(auto sample:audio.output)assert(!sample);}
    parent->setPauseFlag(false);for(unsigned frame=0;frame<4;++frame)audio.step();
    assert(paused.stops==1&&!paused.owner&&pool().getFreeMemCount()==72);
    u32 dead=0;ParentSound invalid;parent=invalid.create();parent->mWaveInfo._24=&dead;assert(parent->play());
    audio.step();assert(invalid.starts==1&&invalid.stops==1&&!invalid.owner&&pool().getFreeMemCount()==72);
    ParentSound disposed;parent=disposed.create();parent->setBankDisposeID(&dead);assert(parent->play());audio.step();
    assert(OSSendMessage(&JASChannel::sBankDisposeMsgQ,&dead,OS_MESSAGE_NOBLOCK));
    JASChannel::receiveBankDisposeMsg();audio.step();
    assert(disposed.starts==1&&disposed.stops==1&&!disposed.owner&&pool().getFreeMemCount()==72);
    JASChannel::receiveBankDisposeMsg();assert(JASDSPChannel::getNumFree()==64);
    std::puts("PASS original parent pause/resume, invalid-wave rejection and SDK bank-disposal message: every terminal path destroys/returns parent once");
}
static void capacity(Harness& audio){
    for(unsigned cycle=0;cycle<10;++cycle){
        std::array<ParentSound,72> sounds{};
        for(unsigned i=0;i<64;++i){auto* parent=sounds[i].create();parent->setInitVolume(.01f);assert(parent->play());}
        for(unsigned i=64;i<72;++i){auto* parent=sounds[i].create();parent->mPriority=0x0101;assert(!parent->play());assert(!sounds[i].owner&&sounds[i].stops==1);}
        assert(pool().getFreeMemCount()==8);audio.step();audio.step();audio.step();audio.step();
        for(const auto& sound:sounds)assert(!sound.owner&&sound.stops==1);
        assert(pool().getFreeMemCount()==72&&JASDSPChannel::getNumFree()==64&&JASDSPChannel::getNumBreak()==0);
    }
    std::puts("PASS original72-parent pool /64 DSP capacity:640 natural and80 rejected parent destructions across10 cycles, all allocations returned");
}
int main(){
    std::setvbuf(stdout,nullptr,_IONBF,0);layout();
    std::vector<u8> storage(0x100000+32);auto base=(reinterpret_cast<std::uintptr_t>(storage.data())+31)&~std::uintptr_t(31);
    JKRExpHeap root(reinterpret_cast<void*>(base),0x100000,nullptr,false);
    JKRHeap::sRootHeap=JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();
    JASDram=JKRSolidHeap::create(0x40000,&root,false);assert(JASDram&&root.check());
    JASChannel::newMemPool(0x48);JASChannel::initBankDisposeMsgQueue();
    {Harness audio;naturalParents(audio);pauseAndDispose(audio);capacity(audio);}
    assert(root.check()&&pool().getTotalMemCount()==72&&pool().getFreeMemCount()==72);
    std::puts("SCOPE: callback-body queue harness; no running JKR audio thread, music stream, device or game. Process-lifetime pool heap intentionally retained: static pool shutdown deallocator is not verified by this test.");
    // Generic pool shutdown currently uses global delete[] for JKR heap chunks.
    // Do not claim shutdown verification or mask it with a test deallocator.
    std::_Exit(0);
}
