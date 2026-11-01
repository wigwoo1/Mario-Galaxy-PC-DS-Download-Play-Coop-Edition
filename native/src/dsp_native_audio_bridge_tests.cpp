#include <Native/DspNativeAudioBridge.hpp>
#include <Native/DspUcodeImage.hpp>
#include <JSystem/JAudio2/JASDspInterface.hpp>
#include <JSystem/JAudio2/JASWaveInfo.hpp>
#include <JSystem/JAudio2/JASCalc.hpp>
#include <JSystem/JAudio2/dspproc.hpp>
#include <JSystem/JAudio2/dsptask.hpp>
#include "native_aram_memory.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <thread>
using Bridge=Native::DspNativeAudioBridge;
using Result=Bridge::Result;
static Bridge* target=nullptr;
static Bridge::Reply latest;
extern "C" void OSReport(const char*,...){}
extern "C" BOOL OSDisableInterrupts(){return 0;}
extern "C" BOOL OSRestoreInterrupts(BOOL state){return state;}
extern "C" void DCFlushRange(void*,u32){}
void JASCalc::bzero(void* memory,u32 size){std::memset(memory,0,size);}
int DSPSendCommands2(u32* words,u32 count,void(*callback)(u16)){
    latest=target->processBatch(words,count);
    if(callback){assert(latest.result==Result::Applied&&latest.count==2);callback(u16(latest.mails[1]));}
    return 0;
}
struct Fixture {
    Native::DspAddressSpace& host=Native::nativeDspAddressSpace();
    std::array<u8,1024> backing{};
    NativeAramMemory aram{backing.data(),backing.size()};
    unsigned reads=0;bool unalignedRead=false;
    static bool read(void* context,u32 offset,void* output,std::size_t bytes){
        auto& fixture=*static_cast<Fixture*>(context);++fixture.reads;
        if(offset%32||bytes%32)fixture.unalignedRead=true;
        return fixture.aram.readSampleBytes(offset,output,bytes);
    }
    Bridge bridge{host,read,this};
    JASDsp::TChannel channels[64]{};
    JASDsp::FxBuf effects[4]{};
    std::array<s16,480> output{}; // Original JAS planar stereo arrangement.
    std::array<s16,160> ring{};
    u32 voices=0,coeffs=0,afc=0,fx=0,left=0,right=0;
    Fixture(){
        host.clear();JASDsp::CH_BUF=channels;JASDsp::FX_BUF=effects;target=&bridge;
        voices=host.map(channels,sizeof(channels));coeffs=host.map(JASDsp::DSPRES_FILTER,sizeof(JASDsp::DSPRES_FILTER));
        afc=host.map(JASDsp::DSPADPCM_FILTER,sizeof(JASDsp::DSPADPCM_FILTER));fx=host.map(effects,sizeof(effects));
        left=host.map(output.data(),sizeof(output));right=host.addressOf(output.data()+240,480);
        output.fill(s16(0x5A5A));
        std::array<u8,448> source{};for(unsigned i=0;i<200;++i){source[32+i*2]=0x27;source[33+i*2]=0x10;}
        const auto token=aram.bindCpu(source.data(),source.size());assert(token&&aram.transfer(0,token,0,source.size()));assert(aram.unbindCpu(token));
    }
    void setup(){DsetVARAM(0xDEADBEEF);DsetupTable(64,voices,coeffs,afc,fx);DsetMixerLevel(1);}
    void activate(unsigned index,bool reverb=false){
        auto& voice=channels[index];voice={};JASWaveInfo info;
        info.mFormat=3;info.mLoopFlags=0;info.mSampleCount=200;info.mSampleLoopStart=0;info.mSampleLoopEnd=200;
        info.mpLastSample=info.mpPenultSample=0;voice.setWaveInfo(info,32,0);voice.playStart();voice.mPitch=0x1000;
        voice.setBusConnect(0,reverb?3:1);voice.setBusConnect(1,reverb?0:2);
        voice._10[0][1]=voice._10[0][2]=0x7FFF;
        if(!reverb)voice._10[1][1]=voice._10[1][2]=0x7FFF;
    }
    void groups(){for(unsigned group=0;group<4;++group){DSPReleaseHalt2(group);assert(latest.result==(group==3?Result::SubframeRendered:Result::Applied));}}
    void enableEffect(unsigned destination=0){
        JASDsp::FxlineConfig_ config{};config._0=4;config._2=destination;config._4=0x7FFF;
        config._6=0;config._8=0;config._C=2;
        assert(JASDsp::setFXLine(0,ring.data(),&config)==1);
        assert(host.addressOf(ring.data(),sizeof(ring))&&effects[0]._4==ring.data());
        assert(reinterpret_cast<std::uintptr_t>(effects[0]._4)>0xFFFFFFFFull);
    }
};
static void nativeVoiceAndOutput(){
    Fixture test;test.setup();test.activate(0);
    std::array<u8,0x80> tail{};std::memcpy(tail.data(),reinterpret_cast<u8*>(&test.channels[0])+0x100,tail.size());
    DsyncFrame2ch(3,test.left,test.right);assert(latest.result==Result::Applied&&latest.count==0);
    for(unsigned frame=0;frame<3;++frame){
        for(unsigned group=0;group<4;++group){
            DSPReleaseHalt2(group);
            assert(latest.result==(group==3?Result::SubframeRendered:Result::Applied));
            if(group<3){assert(latest.count==0);for(unsigned i=0;i<80;++i)assert(test.output[frame*80+i]==s16(0x5A5A));}
        }
        if(frame==0)for(unsigned i=4;i<80;++i)assert(test.output[i]==5001&&test.output[240+i]==5001);
    }
    assert(latest.count==3&&latest.mails[2]==0xDCD10005);
    assert(test.channels[0].mIsFinished&&test.channels[0]._74==0&&test.channels[0]._70==432);
    assert(!std::memcmp(tail.data(),reinterpret_cast<u8*>(&test.channels[0])+0x100,tail.size()));
    assert(test.reads&&test.unalignedRead&&test.bridge.continueAfterFrame());
    std::puts("PASS actual native JAS tables/voices + DMA-uploaded ARAM: exact host-PCM5001, scalar state and read-only-tail preservation, unaligned sample reads, natural EOF and completion after host write-back");
}
static void nativeEffects(){
    Fixture test;test.setup();test.enableEffect();test.activate(0,true);
    DsyncFrame2ch(3,test.left,test.right);
    test.groups();assert(test.ring[10]==5001&&test.output[10]==0);
    test.groups();assert(test.ring[90]==5001&&test.output[90]==0);
    const auto stored=test.ring;
    test.groups();
    // Pinned ApplyReverb prefixes8 history samples even with filtering disabled.
    // The prior read of ring block1 was zero (before its voice contribution).
    // Compare the whole delayed block, including the true resampling startup.
    for(unsigned i=0;i<80;++i){
        const s16 downloaded=i<8?0:s16((s32(stored[i-8])*32767)>>15);
        const s16 expected=s16((s32(downloaded)*32767)>>15);
        assert(test.output[160+i]==expected&&test.output[400+i]==0);
    }
    assert(test.output[178]==4999);
    assert(test.channels[0].mIsFinished&&!test.bridge.failed());
    std::puts("PASS original setFXLine/setBusConnect with64-bit effects pointer: ring registered, BE/host ring write-back, audited two-block delay/8-sample history/read gain and exact returned sample4999");
}
static void rejects(){
    Fixture test;test.setup();test.activate(0);DsyncFrame2ch(3,test.left,test.right);
    const auto before=test.output;DSPReleaseHalt2(1);assert(latest.result==Result::WrongOrder&&latest.count==0&&test.output==before);
    DSPReleaseHalt2(0);assert(test.channels[0]._74==120&&latest.count==0);
    assert(test.host.unmap(test.voices));DSPReleaseHalt2(1);
    assert(latest.result==Result::InvalidMemory&&latest.count==0&&test.bridge.failed()&&test.output==before);
    assert(!test.bridge.continueAfterFrame());
    Fixture effects;effects.setup();effects.enableEffect(2);effects.effects[0]._8=0x1234;
    DsyncFrame2ch(1,effects.left,effects.right);DSPReleaseHalt2(0);
    assert(latest.result==Result::RenderFailed&&latest.count==0&&effects.bridge.failed());
    for(auto sample:effects.output)assert(sample==s16(0x5A5A));
    Fixture unregistered;unregistered.setup();unregistered.enableEffect();
    const auto ring=unregistered.host.addressOf(unregistered.ring.data(),sizeof(unregistered.ring));assert(unregistered.host.unmap(ring));
    DsyncFrame2ch(1,unregistered.left,unregistered.right);DSPReleaseHalt2(0);
    assert(latest.result==Result::InvalidMemory&&latest.count==0&&unregistered.bridge.failed());
    Fixture bounds;bounds.setup();DsyncFrame2ch(3,bounds.left,bounds.right+2);
    assert(latest.result==Result::InvalidMemory&&latest.count==0&&!bounds.bridge.failed());
    std::array<u8,4> read={9,9,9,9};assert(!bounds.aram.readSampleBytes(1022,read.data(),4));for(auto byte:read)assert(byte==9);
    assert(bounds.aram.readSampleBytes(33,read.data(),3));assert(read[0]==0x10&&read[1]==0x27&&read[2]==0x10&&read[3]==9);
    std::puts("PASS stale/unregistered native buffers, wrong group/output bounds and bounded byte reads; unknown bus0x1234 fails visibly with no false completion or output");
}
static void auditedDramRoutes(){
    const auto* bytes=Native::originalDspUcodeImage();assert(Native::originalDspUcodeImageBytes()==0x1F00);
    auto word=[&](unsigned pc){return u16((u16(bytes[pc*2])<<8)|bytes[pc*2+1]);};
    u32 hash=0;for(unsigned i=0;i<0x1F00;++i){hash^=bytes[i];hash=(hash<<3)|(hash>>29);}assert(hash==0xD643001F);
    // Derive DSP DRAM windows/live offsets/count/read gain from real uCode,
    // independently of the renderer's typed arrays and buffer-ID resolver.
    const unsigned first=word(0x0141),stride=word(0x0147)-first;
    const unsigned count=word(0x0157)&255,history=(word(0x013D)&255)*2;
    const unsigned gain=word(0x017F);
    assert(first==0xDC0&&stride==0x60&&count==80&&history==8&&gain==0x7FFF);
    assert(word(0x017A)==0x0508&&word(0x023B)==80);
    for(unsigned filterFlags:{1u,2u,3u,4u}){
    Fixture test;test.setup();std::array<std::array<s16,160>,3> extra{};
    s16* rings[]={test.ring.data(),extra[0].data(),extra[1].data(),extra[2].data()};
    std::array<std::array<s16,160>,4> oracleRings{};
    for(unsigned line=0;line<4;++line){
        assert(word(0x015A+line*4)==first+line*stride+history);
        JASDsp::FxlineConfig_ config{};config._0=filterFlags;config._C=2;
        config._2=2+(line+1)%4;config._4=0x2000;config._6=line%2;config._8=0x4000;
        config._10[0]=0x2000;config._10[3]=-0x0800;config._10[7]=0x1000;
        assert(JASDsp::setFXLine(line,rings[line],&config)==1);
        for(unsigned i=0;i<160;++i)rings[line][i]=oracleRings[line][i]=s16((line%2?-1:1)*(int(line*61+i*3)+17));
    }
    std::array<s16,0x1000> dram{};
    const unsigned left=0xD00,right=0xD60;
    auto add=[&](unsigned to,unsigned from,unsigned volume){
        for(unsigned i=0;i<count;++i)dram[to+i]=s16(dram[to+i]+((s32(dram[from+i])*s32(volume))>>15));
    };
    auto filter=[&](unsigned base){
        for(unsigned i=0;i<count;++i){
            const s64 value=s64(dram[base+i])*0x2000-s64(dram[base+i+3])*0x0800+s64(dram[base+i+7])*0x1000;
            dram[base+i]=s16(std::clamp<s64>(value>>15,-32768,32767));
        }
    };
    DsyncFrame2ch(3,test.left,test.right);
    for(unsigned frame=0;frame<3;++frame){
        for(unsigned i=0;i<count;++i)dram[left+i]=dram[right+i]=0;
        for(unsigned line=0;line<4;++line){
            const unsigned base=first+line*stride;
            for(unsigned i=0;i<history;++i)dram[base+i]=dram[base+count+i];
            for(unsigned i=0;i<count;++i)dram[base+history+i]=0;
        }
        for(unsigned line=0;line<4;++line){
            const unsigned base=first+line*stride;
            for(unsigned i=0;i<count;++i)dram[base+history+i]=s16(dram[base+history+i]+((s32(oracleRings[line][(frame%2)*80+i])*s32(gain))>>15));
            if(filterFlags&1)filter(base);
            add(first+((line+1)%4)*stride+history,base,0x2000);
            add(line%2?right:left,base,0x4000);
            if(filterFlags&2)filter(base);
        }
        for(unsigned line=0;line<4;++line)for(unsigned i=0;i<count;++i)oracleRings[line][(frame%2)*80+i]=dram[first+line*stride+i];
        test.groups();
        for(unsigned i=0;i<count;++i)assert(test.output[frame*80+i]==dram[left+i]&&test.output[240+frame*80+i]==dram[right+i]);
        for(unsigned line=0;line<4;++line)for(unsigned i=0;i<160;++i)assert(rings[line][i]==oracleRings[line][i]);
    }
    assert(latest.count==3&&!test.bridge.failed());
    }
    std::puts("PASS D643001F uCode-anchored absolute-DRAM oracle: all4 original SEND_TABLE live-offset routes, signed feedback/history, pre/post/both/no FIR, every stereo sample and4 ring states over3 subframes");
}
static void concurrentSampleRead(){
    std::array<u8,128> backing{};std::array<u8,64> a{},b{};a.fill(0x11);b.fill(0x77);
    NativeAramMemory memory(backing.data(),backing.size());
    const auto first=memory.bindCpu(a.data(),a.size()),second=memory.bindCpu(b.data(),b.size());
    assert(first&&second&&memory.transfer(0,first,32,64));
    std::thread dma([&]{for(unsigned i=0;i<2000;++i)assert(memory.transfer(0,i%2?first:second,32,64));});
    for(unsigned i=0;i<2000;++i){
        std::array<u8,61> copy{};assert(memory.readSampleBytes(33,copy.data(),copy.size()));
        assert(copy[0]==0x11||copy[0]==0x77);for(auto byte:copy)assert(byte==copy[0]);
    }
    dma.join();assert(memory.unbindCpu(first)&&memory.unbindCpu(second));
    std::puts("PASS concurrent DMA/sample access:2000 unaligned snapshots contain complete transfers, with DMA alignment/token rules preserved");
}
int main(){std::setvbuf(stdout,nullptr,_IONBF,0);nativeVoiceAndOutput();nativeEffects();rejects();auditedDramRoutes();concurrentSampleRead();}
