#include <Native/DspAudioFrameExecutor.hpp>
#include <Native/DspVoiceWire.hpp>
#include <JSystem/JAudio2/JASWaveInfo.hpp>
#include <JSystem/JAudio2/dspproc.hpp>
#include <JSystem/JAudio2/dsptask.hpp>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>

using Executor=Native::DspAudioFrameExecutor;
static Executor* target=nullptr;
static Executor::Reply latest;
static std::vector<u32> packet;
extern "C" void OSReport(const char*,...){}
int DSPSendCommands2(u32* words,u32 count,void(*callback)(u16)){
    packet.assign(words,words+(count?count:1));
    latest=target->processBatch(words,count);
    // Only the original synchronous table/VARAM setup uses this test hook.
    // Actual mailbox/IRQ dispatch and audio-thread callbacks are not replaced
    // by this hook in production: this executable is not linked into the game.
    if(callback){assert(latest.result==Executor::Result::Applied&&latest.count==2);callback(u16(latest.mails[1]));}
    return 0;
}
struct Fixture {
    Native::DspAddressSpace addresses;
    Native::ZeldaAudioMemory memory{addresses};
    Executor executor{memory};
    JASDsp::TChannel channels[64]{};
    std::array<unsigned char,64*0x180> voices{};
    std::array<unsigned char,0x500> coefficients{};
    std::array<unsigned char,0x80> reverb{};
    std::array<unsigned char,400> samples{};
    std::vector<unsigned char> left=std::vector<unsigned char>(255*160,0x5A),right=left;
    u32 voiceAddress=addresses.map(voices.data(),voices.size());
    u32 coeffAddress=addresses.map(coefficients.data(),coefficients.size());
    u32 afcAddress=addresses.map(JASDsp::DSPADPCM_FILTER,sizeof(JASDsp::DSPADPCM_FILTER));
    u32 reverbAddress=addresses.map(reverb.data(),reverb.size());
    u32 sampleAddress=addresses.map(samples.data(),samples.size());
    u32 leftAddress=addresses.map(left.data(),left.size()),rightAddress=addresses.map(right.data(),right.size());
    Fixture(){
        target=&executor;JASDsp::CH_BUF=channels;
        for(unsigned i=0;i<320;++i)for(unsigned b=0;b<4;++b)coefficients[i*4+b]=JASDsp::DSPRES_FILTER[i]>>(24-b*8);
        for(unsigned i=0;i<200;++i){samples[i*2]=0x27;samples[i*2+1]=0x10;}
    }
    void setup(){
        DsetVARAM(sampleAddress);assert(latest.mails[0]==0xDCD10004&&latest.mails[1]==0xF3558E00);
        DsetupTable(64,voiceAddress,coeffAddress,afcAddress,reverbAddress);
        assert(latest.mails[1]==0xF3558100&&executor.state()==Executor::State::Idle);
        DsetMixerLevel(1);
    }
    void activate(unsigned index,bool loop=false){
        auto& voice=channels[index];voice={};JASWaveInfo info;
        info.mFormat=3;info.mLoopFlags=loop;info.mSampleCount=200;
        info.mSampleLoopStart=40;info.mSampleLoopEnd=200;info.mpLastSample=info.mpPenultSample=0;
        voice.setWaveInfo(info,0,0);voice.playStart();voice.mPitch=0x1000;
        voice._10[0][0]=0xD00;voice._10[1][0]=0xD60;
        for(unsigned ch=0;ch<2;++ch)voice._10[ch][1]=voice._10[ch][2]=256;
    }
    void group(unsigned index){
        for(unsigned i=index*16;i<(index+1)*16;++i)assert(Native::encodeDspVoice(channels[i],voices.data()+i*0x180,0x180));
        DSPReleaseHalt2(index);
        assert(packet[0]==((index<<16)|DSP_CreateMap2(index)));
        for(unsigned i=index*16;i<(index+1)*16;++i)assert(Native::decodeDspVoiceState(voices.data()+i*0x180,0x180,channels[i]));
    }
    s16 output(unsigned sample,bool rhs=false){
        const auto& bytes=rhs?right:left;const u16 bits=(u16(bytes[sample*2])<<8)|bytes[sample*2+1];
        s16 value;std::memcpy(&value,&bits,2);return value;
    }
};
static void realCommands(){
    Fixture test;test.setup();
    for(unsigned i=0;i<64;++i)test.activate(i);
    DsyncFrame2ch(3,test.leftAddress,test.rightAddress);
    assert(latest.count==0&&latest.result==Executor::Result::Applied);
    assert(!test.executor.continueAfterFrame());
    for(unsigned frame=0;frame<3;++frame){
        for(unsigned group=0;group<4;++group){
            const auto previous=test.channels[63]._74;
            test.group(group);
            if(group<3){
                assert(latest.result==Executor::Result::Applied&&latest.count==0);
                for(unsigned i=frame*160;i<(frame+1)*160;++i)assert(test.left[i]==0x5A&&test.right[i]==0x5A);
                assert(test.channels[63]._74==previous);
            }else{
                assert(latest.result==Executor::Result::SubframeRendered);
                assert(latest.count==(frame==2?3:2));
                assert(latest.mails[0]==0xDCD10004&&latest.mails[1]==(0xF355FF00|frame));
                assert(test.executor.completedSubframes()==frame+1);
            }
            for(unsigned i=group*16;i<(group+1)*16;++i){
                assert(test.channels[i]._74==(frame==0?120:frame==1?40:0));
                assert(bool(test.channels[i].mIsFinished)==(frame==2));
            }
            if(group==0){
                auto before=test.voices;
                const u32 duplicate=0x0000FFFF,skipped=0x0002FFFF;
                assert(test.executor.processBatch(&duplicate,0).result==Executor::Result::WrongOrder);
                assert(test.executor.processBatch(&skipped,0).result==Executor::Result::WrongOrder);
                assert(test.voices==before&&test.executor.nextGroup()==1);
            }
        }
        // 64 voices * ((10003 resampled amplitude *256 gain)>>16).
        if(frame==0)for(unsigned i=4;i<80;++i)assert(test.output(i)==2496&&test.output(i,true)==2496);
    }
    assert(latest.mails[2]==0xDCD10005&&test.executor.state()==Executor::State::WaitingContinue);
    DsyncFrame2ch(1,test.leftAddress,test.rightAddress);assert(latest.result==Executor::Result::WrongOrder&&latest.count==0);
    assert(test.executor.continueAfterFrame()&&!test.executor.continueAfterFrame());
    // The CPU parameter area must survive DSP state write-back unchanged.
    for(auto& voice:test.channels)assert(voice._11C==200&&voice._114==200);
    std::puts("PASS original setup/frame/map/group producers:64 voices advance exactly once per subframe; exact PCM2496; EOF after3 subframes; ordered completion and frame-end/continue gate");
}
static void masksAndFrameBounds(){
    Fixture test;test.setup();test.activate(63,true);test.activate(0);test.channels[0].mIsActive=0;
    DsyncFrame2ch(3,test.leftAddress,test.rightAddress);
    for(unsigned frame=0;frame<3;++frame)for(unsigned group=0;group<4;++group)test.group(group);
    assert(test.channels[0]._8==1&&test.channels[0]._74==0); // Never rendered, still reset.
    assert(!test.channels[63].mIsFinished&&test.channels[63]._70==160&&test.channels[63]._74==120);
    assert(test.executor.continueAfterFrame());
    // The full advertised buffer extent must be valid before any rendering.
    DsyncFrame2ch(3,test.leftAddress,test.rightAddress+u32(test.right.size())-160);
    assert(latest.result==Executor::Result::InvalidMemory&&latest.count==0&&test.executor.state()==Executor::State::Idle);
    DsyncFrame2ch(0,test.leftAddress,test.rightAddress);assert(latest.result==Executor::Result::Malformed&&latest.count==0);
    test.channels[63].mPauseFlag=1;
    // The original renderer holds the last sample while JAS's pause flag is
    // set; it does not promise silence. Verify that exact constant and freeze.
    const s16 held=s16((s32(test.channels[63]._66)*256)>>16);
    DsyncFrame2ch(255,test.leftAddress,test.rightAddress);
    for(unsigned frame=0;frame<255;++frame){
        for(unsigned group=0;group<4;++group)test.group(group);
        assert(latest.result==Executor::Result::SubframeRendered&&latest.mails[1]==(0xF355FF00|frame));
        for(unsigned i=0;i<80;++i)assert(test.output(frame*80+i)==held&&test.output(frame*80+i,true)==held);
    }
    assert(test.executor.completedSubframes()==255&&latest.count==3&&test.channels[63]._74==120);
    std::puts("PASS sparse original maps, loop continuity, pause, maximum255-subframe request, full output bounds, zero-frame rejection and no premature completion");
}
static void failureGates(){
    Fixture test;
    DsyncFrame2ch(1,test.leftAddress,test.rightAddress);assert(latest.result==Executor::Result::WrongOrder&&latest.count==0);
    const u32 badTables[]={0x81000040,test.voiceAddress,test.coeffAddress,1,test.reverbAddress};
    assert(test.executor.processBatch(badTables,5).result==Executor::Result::InvalidMemory);
    assert(test.executor.state()==Executor::State::Idle);
    test.setup();test.activate(0);test.channels[0]._118=0xFFFFFF00u;
    DsyncFrame2ch(1,test.leftAddress,test.rightAddress);test.group(0);
    assert(latest.result==Executor::Result::RenderFailed&&latest.count==0);
    assert(test.executor.state()==Executor::State::Faulted&&!test.executor.continueAfterFrame());
    for(unsigned char value:test.left)assert(value==0x5A);
    const u32 group=0;assert(test.executor.processBatch(&group,0).result==Executor::Result::WrongOrder);
    assert(test.executor.processBatch(nullptr,5).result==Executor::Result::Malformed);
    const u32 unsupported=0x550001;assert(test.executor.processBatch(&unsupported,0).result==Executor::Result::Unsupported);
    Fixture badEffects;badEffects.setup();badEffects.reverb[1]=1;
    DsyncFrame2ch(1,badEffects.leftAddress,badEffects.rightAddress);badEffects.group(0);
    assert(latest.result==Executor::Result::RenderFailed&&latest.count==0&&badEffects.executor.state()==Executor::State::Faulted);
    std::puts("PASS setup/order/memory/unsupported gates and real renderer failures: no completion mail on failure, partial-render retries prohibited");
}
int main(){std::setvbuf(stdout,nullptr,_IONBF,0);realCommands();masksAndFrameBounds();failureGates();}
