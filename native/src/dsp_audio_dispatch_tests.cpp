#include <Native/DspAudioDispatch.hpp>
#include <Native/DspMailbox.hpp>
#include <JSystem/JAudio2/JASDspInterface.hpp>
#include <JSystem/JAudio2/JASWaveInfo.hpp>
#include <JSystem/JAudio2/JASCalc.hpp>
#include <JSystem/JAudio2/dspproc.hpp>
#include <JSystem/JAudio2/dsptask.hpp>
#include <revolution/dsp.h>
#include "native_aram_memory.hpp"
#include "dsp_audio_dispatch_test_callback.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>
void DspInitWork();int DspCanStartWork();
void DsyncFrame2(u32,u32,u32);
TestAudioThread* TestAudioThread::current=nullptr;
static unsigned taskErrors=0;
extern "C" void OSReport(const char* format,...){if(std::strstr(format,"Not finish task"))++taskErrors;}
extern "C" void DCFlushRange(void*,u32){}
void JASCalc::bzero(void* data,u32 bytes){std::memset(data,0,bytes);}
struct Fixture {
    Native::DspAddressSpace& host=Native::nativeDspAddressSpace();
    TestAudioThread audio;
    std::array<u8,1024> backing{};
    NativeAramMemory aram{backing.data(),backing.size()};
    static bool read(void* context,u32 offset,void* output,std::size_t bytes){return static_cast<Fixture*>(context)->aram.readSampleBytes(offset,output,bytes);}
    Native::DspNativeAudioBridge bridge{host,read,this};
    Native::DspAudioDispatch dispatcher{bridge,TestAudioThread::DSPCallback};
    JASDsp::TChannel channels[64]{};JASDsp::FxBuf effects[4]{};
    std::array<s16,480> output{};
    u32 voices=0,coeffs=0,afc=0,fx=0,left=0,right=0;
    static s32 process(void* context,const u32* words,u32 count,u16* token){return static_cast<Fixture*>(context)->dispatcher.process(words,count,token);}
    static void service(void* context){static_cast<Fixture*>(context)->dispatcher.service();}
    Fixture(){
        DSPInit();host.clear();DspBoot(nullptr); // Actual verified ROM handshake and original task initialization.
        JASDsp::CH_BUF=channels;JASDsp::FX_BUF=effects;
        voices=host.map(channels,sizeof(channels));coeffs=host.map(JASDsp::DSPRES_FILTER,sizeof(JASDsp::DSPRES_FILTER));
        afc=host.map(JASDsp::DSPADPCM_FILTER,sizeof(JASDsp::DSPADPCM_FILTER));fx=host.map(effects,sizeof(effects));
        left=host.map(output.data(),sizeof(output));right=host.addressOf(output.data()+240,480);
        output.fill(s16(0x5A5A));
        std::array<u8,416> samples{};for(unsigned i=0;i<200;++i){samples[i*2]=0x27;samples[i*2+1]=0x10;}
        const u32 token=aram.bindCpu(samples.data(),samples.size());assert(token&&aram.transfer(0,token,0,samples.size())&&aram.unbindCpu(token));
        nativeSetDspHostAudioBackend(process,service,this);
    }
    ~Fixture(){nativeSetDspHostAudioBackend(nullptr,nullptr,nullptr);DSPInit();}
    void setup(){DsetVARAM(0);DsetupTable(64,voices,coeffs,afc,fx);DsetMixerLevel(1);assert(!dispatcher.pending()&&!dispatcher.failed());}
    void activate(){
        JASWaveInfo info;info.mFormat=3;info.mLoopFlags=0;info.mSampleCount=200;info.mSampleLoopStart=0;info.mSampleLoopEnd=200;info.mpLastSample=info.mpPenultSample=0;
        channels[0].setWaveInfo(info,0,0);channels[0].playStart();channels[0].mPitch=0x1000;
        channels[0].setBusConnect(0,1);channels[0].setBusConnect(1,2);
        for(unsigned side=0;side<2;++side)channels[0]._10[side][1]=channels[0]._10[side][2]=0x7FFF;
    }
    bool receive(){OSMessage message=nullptr;if(!OSReceiveMessage(&audio.queue,&message,OS_MESSAGE_NOBLOCK))return false;assert(std::intptr_t(message)==AUDIOMSG_DSP);return true;}
};
static std::vector<u16> completed;
static void callback(u16 slot){completed.push_back(slot);}
static bool reenter=false;
static void reentrant(u16 slot){
    completed.push_back(slot);
    if(!reenter){reenter=true;u32 packet[]={0x8E000000,0};assert(DSPSendCommands2(packet,2,reentrant)>0);}
}
static void completionQueue(){
    Fixture test;completed.clear();u32 packet[]={0x8E000000,0};
    u32 malformed[]={0x8E000000,0,0};assert(DSPSendCommands2(malformed,3,callback)==-1&&completed.empty());
    const auto enabled=OSDisableInterrupts();assert(enabled);
    for(unsigned i=0;i<15;++i)assert(DSPSendCommands2(packet,2,callback)>0);
    assert(!DspCanStartWork()&&completed.empty()&&test.dispatcher.pending()==30);
    const auto pending=test.dispatcher.pending();assert(DSPSendCommands2(packet,2,callback)==-1&&test.dispatcher.pending()==pending);
    nativeDspServiceHostAudio();assert(completed.empty()&&test.dispatcher.pending()==30);
    OSRestoreInterrupts(enabled);nativeDspServiceHostAudio();
    assert(completed.size()==15&&test.dispatcher.pending()==0&&DspCanStartWork());
    for(unsigned i=0;i<15;++i)assert(completed[i]==i);
    completed.clear();reenter=false;assert(DSPSendCommands2(packet,2,reentrant)>0);
    assert(completed.size()==2&&completed[0]==15&&completed[1]==0&&test.dispatcher.pending()==0&&!test.dispatcher.failed());
    assert(!DSPCheckMailFromDSP()&&!taskErrors);
    std::puts("PASS real ROM boot/SDK mailbox/original task queue and JAudio callback: deferred critical-section delivery,15-task backpressure without render acceptance, queue wrap, reentrant callback and malformed rejection");
}
static void actualSubframes(){
    Fixture test;test.setup();test.activate();DsyncFrame2(3,test.left,test.right);assert(!test.receive());
    for(unsigned frame=0;frame<3;++frame){
        for(unsigned group=0;group<4;++group){
            DSPReleaseHalt2(group);
            if(group<3)assert(!test.receive());
            else{
                assert(test.receive()&&!test.receive());
                assert(test.dispatcher.subframeNotifications()==frame+1);
                if(!frame)for(unsigned i=4;i<80;++i)assert(test.output[i]==5001&&test.output[240+i]==5001);
            }
        }
    }
    assert(test.channels[0].mIsFinished&&test.dispatcher.frameEnds()==1&&!test.dispatcher.failed());
    assert(test.bridge.state()==Native::DspAudioFrameExecutor::State::Idle);
    // FRAME_END/continue must run before a queued audio-thread update can submit
    // the next frame. This exercises the original deferred frame entry again.
    DsyncFrame2(1,test.left,test.right);
    for(unsigned group=0;group<4;++group)DSPReleaseHalt2(group);
    assert(test.receive()&&!test.receive()&&test.dispatcher.frameEnds()==2);
    for(unsigned i=0;i<80;++i)assert(test.output[i]==0&&test.output[240+i]==0);
    assert(!DSPCheckMailFromDSP()&&!taskErrors);
    std::puts("PASS actual command/send/deferred/map/renderer/SDK callback/message-queue path:3 rendered subframe notices after host PCM/state, natural EOF and frame-end continue before the next frame");
}
static void callbackMustConsume(void*){}
static void resetAndFailure(){
    {
        Fixture test;completed.clear();u32 packet[]={0x8E000000,0};
        const auto enabled=OSDisableInterrupts();assert(DSPSendCommands2(packet,2,callback)>0);
        OSRestoreInterrupts(enabled);assert(test.dispatcher.pending()==2&&completed.empty());
        // Teardown unregisters without delivering old callbacks.
    }
    {
        Fixture test;u32 packet[]={0x8E000000,0};
        assert(DSPSendCommands2(packet,2,callback)==1&&completed.size()==1&&completed[0]==0);
    }
    {
        Fixture test;
        Native::DspAudioDispatch broken(test.bridge,callbackMustConsume);
        nativeSetDspHostAudioBackend([](void* context,const u32* words,u32 count,u16* token){return static_cast<Native::DspAudioDispatch*>(context)->process(words,count,token);},[](void* context){static_cast<Native::DspAudioDispatch*>(context)->service();},&broken);
        u32 packet[]={0x8E000000,0};assert(DSPSendCommands2(packet,2,nullptr)==0);
        assert(broken.failed()&&DSPCheckMailFromDSP());
        nativeSetDspHostAudioBackend(nullptr,nullptr,nullptr);
    }
    std::puts("PASS restart resets original task indices; pending teardown has no late callbacks; non-consuming callback leaves visible failure/mail, without fabricated completion");
}
int main(){std::setvbuf(stdout,nullptr,_IONBF,0);completionQueue();actualSubframes();resetAndFailure();}
