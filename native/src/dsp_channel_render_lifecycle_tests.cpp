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
JKRSolidHeap* JASDram=nullptr;
struct Sound {
    unsigned starts=0,plays=0,stops=0,drops=0;
    JASDSPChannel* owner=nullptr;
    u32 samples=200;bool loop=false;s16 gain=0x7FFF;
    static s32 callback(u32 event,JASDsp::TChannel* voice,void* context){
        auto& sound=*static_cast<Sound*>(context);
        switch(event){
        case JASDSPChannel::CB_START:{
            ++sound.starts;assert(voice);
            JASWaveInfo info;info.mFormat=3;info.mLoopFlags=sound.loop;info.mSampleCount=sound.samples;
            info.mSampleLoopStart=40;info.mSampleLoopEnd=sound.samples;info.mpLastSample=info.mpPenultSample=0;
            voice->setWaveInfo(info,0x4000,0);voice->setPitch(0x1000);voice->setFilterMode(0);
            voice->setBusConnect(0,1);voice->setBusConnect(1,2);
            voice->setMixerInitVolume(0,sound.gain);voice->setMixerInitVolume(1,sound.gain);
            return 0;
        }
        case JASDSPChannel::CB_PLAY:++sound.plays;assert(voice);return 0;
        // Match the required owner release in the original JASChannel
        // dspUpdateCallback STOP/DROP cases. This harness owns its context;
        // it does not instantiate or claim to test the parent JASChannel yet.
        case JASDSPChannel::CB_STOP:++sound.stops;assert(!voice&&sound.owner);sound.owner->free();sound.owner=nullptr;return -1;
        case JASDSPChannel::CB_DROP:++sound.drops;assert(!voice&&sound.owner);sound.owner->free();sound.owner=nullptr;return -1;
        }
        assert(false);return -1;
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
        // Solid-heap allocations are reclaimed together by its real destruction.
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
static void naturalCleanup(Harness& audio){
    Sound sound;auto* channel=JASDSPChannel::alloc(20,Sound::callback,&sound);assert(channel);sound.owner=channel;channel->start();
    audio.step();assert(sound.starts==1&&sound.stops==0&&JASDSPChannel::getNumUse()==1);
    for(unsigned i=4;i<80;++i)assert(audio.output[i]==5001&&audio.output[80+i]==5001);
    assert(channel->mChannel->_74==120&&!channel->mChannel->isFinish());
    audio.step();assert(channel->mChannel->_74==40&&!channel->mChannel->isFinish());
    audio.step();assert(channel->mChannel->isFinish()&&sound.stops==0);
    // The original CPU owner observes natural DSP EOF on its next update.
    audio.step();assert(sound.stops==1&&channel->mPriority==-1);
    assert(channel->getStatus()==JASDSPChannel::STATUS_INACTIVE&&!channel->mChannel->isActive()&&!channel->mChannel->isFinish());
    assert(JASDSPChannel::getNumUse()==0&&JASDSPChannel::getNumFree()==64);
    for(auto sample:audio.output)assert(sample==0);
    std::puts("PASS complete original heap-backed DSP channel start/updateAll + real render EOF: STOP callback, finished reply, priority/active state and all64 slots free without fabricated completion flags");
}
static void replacementHandshake(Harness& audio){
    Sound old;old.loop=true;auto* channel=JASDSPChannel::alloc(20,Sound::callback,&old);assert(channel);old.owner=channel;channel->start();
    for(unsigned i=0;i<4;++i)audio.step();assert(!channel->mChannel->isFinish());
    channel->drop();assert(old.drops==1&&channel->getStatus()==JASDSPChannel::STATUS_DROP);
    Sound replacement;auto* reused=JASDSPChannel::alloc(21,Sound::callback,&replacement);assert(reused==channel);replacement.owner=reused;reused->start();
    unsigned finishedAt=0,startedAt=0;
    for(unsigned frame=1;frame<=40;++frame){
        audio.step();
        if(!finishedAt&&reused->mChannel->isFinish())finishedAt=frame;
        if(replacement.starts){startedAt=frame;break;}
        assert(replacement.stops==0);
    }
    assert(finishedAt>1&&startedAt==finishedAt+2&&old.stops==0&&old.drops==1);
    assert(replacement.starts==1&&replacement.stops==0&&reused->getStatus()==JASDSPChannel::STATUS_ACTIVE);
    for(unsigned i=0;i<3;++i)audio.step();
    assert(replacement.stops==1&&JASDSPChannel::getNumUse()==0&&JASDSPChannel::getNumFree()==64);
    std::printf("PASS real drop/replacement handshake: original forceStop fade completes after%u subframes; replacement starts at%u and later receives its own natural STOP\n",finishedAt,startedAt);
}
static void fullCapacityReuse(Harness& audio){
    for(unsigned cycle=0;cycle<10;++cycle){
        std::array<Sound,64> sounds{};
        for(auto& sound:sounds){sound.gain=256;auto* channel=JASDSPChannel::alloc(20,Sound::callback,&sound);assert(channel);sound.owner=channel;channel->start();}
        assert(!JASDSPChannel::alloc(19,Sound::callback,&sounds[0]));
        audio.step();assert(JASDSPChannel::getNumUse()==64);
        for(unsigned i=4;i<80;++i)assert(audio.output[i]==2496&&audio.output[80+i]==2496);
        audio.step();audio.step();audio.step();
        for(const auto& sound:sounds)assert(sound.starts==1&&sound.stops==1&&sound.drops==0);
        assert(JASDSPChannel::getNumUse()==0&&JASDSPChannel::getNumFree()==64&&JASDSPChannel::getNumBreak()==0);
    }
    std::puts("PASS original64-channel capacity/priority pressure and10 full reuse cycles:640 natural STOP callbacks, exact mixed PCM2496 and no retained DSP channels");
}
int main(){
    std::setvbuf(stdout,nullptr,_IONBF,0);
    std::vector<u8> storage(0x100000+32);auto base=(reinterpret_cast<std::uintptr_t>(storage.data())+31)&~std::uintptr_t(31);
    JKRExpHeap root(reinterpret_cast<void*>(base),0x100000,nullptr,false);
    JKRHeap::sRootHeap=JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();const auto initial=root.getTotalFreeSize();
    JASDram=JKRSolidHeap::create(0x40000,&root,false);assert(JASDram&&root.check());
    {Harness audio;naturalCleanup(audio);replacementHandshake(audio);fullCapacityReuse(audio);}
    JKRHeap::destroy(JASDram);JASDram=nullptr;
    assert(root.check()&&root.getTotalFreeSize()==initial);
    std::puts("PASS real root/solid heap ownership and complete audio test allocation teardown");
}
