#include <Native/DspAudioRuntime.hpp>
#include <Native/DspAddressSpace.hpp>
#include <Native/DspMailbox.hpp>
#include <JSystem/JAudio2/JASDspInterface.hpp>
#include <JSystem/JAudio2/JASWaveInfo.hpp>
#include <JSystem/JAudio2/JASCalc.hpp>
#include <JSystem/JAudio2/dspproc.hpp>
#include <JSystem/JAudio2/dsptask.hpp>
#include <revolution/dsp.h>
#include "native_jkr_aram_port.hpp"
#include "dsp_audio_dispatch_test_callback.hpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <stdexcept>
void DsyncFrame2(u32,u32,u32);
TestAudioThread* TestAudioThread::current=nullptr;
static unsigned taskErrors=0;
extern "C" void OSReport(const char* format,...){if(std::strstr(format,"Not finish task"))++taskErrors;}
extern "C" void DCFlushRange(void*,u32){}
void JASCalc::bzero(void* data,u32 bytes){std::memset(data,0,bytes);}
struct Fixture {
    TestAudioThread audio;
    std::array<u8,1024> backing{};
    std::shared_ptr<NativeAramMemory> memory=std::make_shared<NativeAramMemory>(backing.data(),backing.size());
    JASDsp::TChannel channels[64]{};JASDsp::FxBuf effects[4]{};
    std::array<s16,480> output{};
    u32 voices=0,coeffs=0,afc=0,fx=0,left=0,right=0;
    Fixture(){
        DSPInit();nativeReleaseAramTransport();Native::nativeDspAddressSpace().clear();
        assert(nativeBindAramTransport(memory));
        std::array<u8,416> samples{};for(unsigned i=0;i<200;++i){samples[i*2]=0x27;samples[i*2+1]=0x10;}
        assert(nativeUploadAram(0,samples.data(),samples.size()));
        nativeConfigureDspAudioRuntime();
        DspBoot(TestAudioThread::DSPCallback);
        assert(nativeDspAudioRuntimeActive()&&!nativeDspAudioRuntimeFailed());
        assert(nativeDspBootRequestCallback()==TestAudioThread::DSPCallback);
        auto& host=Native::nativeDspAddressSpace();JASDsp::CH_BUF=channels;JASDsp::FX_BUF=effects;
        voices=host.map(channels,sizeof(channels));coeffs=host.map(JASDsp::DSPRES_FILTER,sizeof(JASDsp::DSPRES_FILTER));
        afc=host.map(JASDsp::DSPADPCM_FILTER,sizeof(JASDsp::DSPADPCM_FILTER));fx=host.map(effects,sizeof(effects));
        left=host.map(output.data(),sizeof(output));right=host.addressOf(output.data()+240,480);
        output.fill(s16(0x5A5A));setup();
    }
    ~Fixture(){nativeStopDspAudioRuntime();nativeSetDspAudioBootHooks(nullptr,nullptr);DSPInit();nativeReleaseAramTransport();}
    void setup(){DsetVARAM(0);DsetupTable(64,voices,coeffs,afc,fx);DsetMixerLevel(1);}
    void activate(){
        auto& voice=channels[0];voice={};JASWaveInfo info;
        info.mFormat=3;info.mLoopFlags=0;info.mSampleCount=200;info.mSampleLoopStart=0;info.mSampleLoopEnd=200;info.mpLastSample=info.mpPenultSample=0;
        voice.setWaveInfo(info,0,0);voice.playStart();voice.mPitch=0x1000;voice.setBusConnect(0,1);voice.setBusConnect(1,2);
        for(unsigned side=0;side<2;++side)voice._10[side][1]=voice._10[side][2]=0x7FFF;
    }
    bool receive(){OSMessage message=nullptr;if(!OSReceiveMessage(&audio.queue,&message,OS_MESSAGE_NOBLOCK))return false;assert(std::intptr_t(message)==AUDIOMSG_DSP);return true;}
    void groups(){for(unsigned group=0;group<4;++group)DSPReleaseHalt2(group);}
};
static void automaticActivation(){
    Fixture test;test.activate();DsyncFrame2(3,test.left,test.right);
    const auto interrupts=OSDisableInterrupts();test.groups();
    assert(!test.receive());nativeDspServiceHostAudio();assert(!test.receive());
    OSRestoreInterrupts(interrupts);
    nativeDspServiceHostAudio(); // Same service boundary used before audio-loop wait.
    assert(test.receive()&&!test.receive());
    for(unsigned i=4;i<80;++i)assert(test.output[i]==5001&&test.output[240+i]==5001);
    test.groups();assert(test.receive());test.groups();assert(test.receive()&&!test.receive());
    assert(test.channels[0].mIsFinished&&!nativeDspAudioRuntimeFailed());
    DSPInit();assert(!nativeDspAudioRuntimeActive()&&!nativeDspBootRequestCallback());
    DspBoot(TestAudioThread::DSPCallback);assert(nativeDspAudioRuntimeActive());test.setup();test.activate();
    DsyncFrame2(1,test.left,test.right);test.groups();assert(test.receive()&&!test.receive());
    assert(!nativeDspAudioRuntimeFailed()&&!taskErrors);
    std::puts("PASS automatic verified-boot callback/real-ARAM binding, original setup callbacks, actual PCM5001 and EOF notices, outer-critical startup servicing and DSP reset/reboot activation");
}
static void pendingStop(){
    Fixture test;test.activate();DsyncFrame2(1,test.left,test.right);
    const auto interrupts=OSDisableInterrupts();test.groups();OSRestoreInterrupts(interrupts);
    assert(!test.receive());nativeStopDspAudioRuntime();assert(!nativeDspAudioRuntimeActive());
    nativeDspServiceHostAudio();assert(!test.receive());
    DspBoot(TestAudioThread::DSPCallback);test.setup();test.activate();DsyncFrame2(1,test.left,test.right);test.groups();
    assert(test.receive()&&!test.receive()&&!nativeDspAudioRuntimeFailed());
    std::puts("PASS owned-runtime stop unregisters before destruction, discards pending notices without late callbacks and supports fresh boot");
}
static void transportLifetime(){
    Fixture test;test.activate();DsyncFrame2(1,test.left,test.right);
    std::weak_ptr<NativeAramMemory> old=test.memory;
    nativeReleaseAramTransport();test.memory.reset();assert(!old.expired()); // Runtime owns the context.
    auto replacement=std::make_shared<NativeAramMemory>(test.backing.data(),test.backing.size());
    assert(nativeBindAramTransport(replacement));
    std::array<u8,8> probe{};assert(!nativeReadBoundAramSamples(old.lock().get(),0,probe.data(),probe.size()));
    DSPReleaseHalt2(0);assert(nativeDspAudioRuntimeFailed()&&!test.receive());
    for(auto value:test.output)assert(value==s16(0x5A5A));
    nativeStopDspAudioRuntime();assert(old.expired());
    nativeReleaseAramTransport();bool rejected=false;
    try{DspBoot(TestAudioThread::DSPCallback);}catch(const std::runtime_error&){rejected=true;}
    assert(rejected&&!nativeDspAudioRuntimeActive());
    std::puts("PASS released/replaced ARAM identity is rejected before reading, old context retained only until stop, no false notice/output and missing transport fails boot visibly");
}
int main(){std::setvbuf(stdout,nullptr,_IONBF,0);automaticActivation();pendingStop();transportLifetime();}
