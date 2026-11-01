#include "assets.hpp"
#include <Native/ZeldaAudioRenderer.hpp>
#include <Native/DspVoiceWire.hpp>
#include <revolution/dvd.h>
#include <Native/JasStreamPcm.hpp>
#include <JSystem/JAudio2/JASDspInterface.hpp>
#include <JSystem/JAudio2/JASWaveInfo.hpp>
#include <cassert>
#include <cstdio>
#include <fstream>

static std::array<Native::s16,0x100> coefficients(){
    std::array<Native::s16,0x100> result{};
    for(unsigned i=0;i<result.size();++i){const auto bits=Native::u16(JASDsp::DSPRES_FILTER[i/2]>>(i%2?0:16));std::memcpy(&result[i],&bits,2);}
    return result;
}
template<unsigned Start,unsigned Count>static std::array<Native::s16,Count> tableWords(){
    static_assert(Start+Count<=640);std::array<Native::s16,Count> result{};
    for(unsigned i=0;i<Count;++i){const unsigned index=Start+i;const auto bits=Native::u16(JASDsp::DSPRES_FILTER[index/2]>>(index%2?0:16));std::memcpy(&result[i],&bits,2);}return result;
}
static std::array<Native::s16,32> afcCoefficients(){
    std::array<Native::s16,32> result{};for(unsigned i=0;i<32;++i){const auto* p=JASDsp::DSPADPCM_FILTER+i*2;const Native::u16 bits=(Native::u16(p[0])<<8)|p[1];std::memcpy(&result[i],&bits,2);}return result;
}
struct RendererFixture {
    Native::DspAddressSpace addresses;
    Native::ZeldaAudioMemory memory{addresses};
    Native::ZeldaAudioRenderer renderer{memory};
    std::array<unsigned char,64*Native::dspVoiceBytes> table{};
    std::array<unsigned char,160> left{},right{};
    std::array<unsigned char,128> reverb{}; // Four real disabled wire-format RPBs.
    Native::u32 voices=addresses.map(table.data(),table.size());
    Native::u32 leftAddress=addresses.map(left.data(),left.size()),rightAddress=addresses.map(right.data(),right.size());
    RendererFixture(){renderer.SetVPBBaseAddress(voices);renderer.SetReverbPBBaseAddress(addresses.map(reverb.data(),reverb.size()));renderer.SetResamplingCoeffs(coefficients());renderer.SetConstPatterns(tableWords<256,256>());renderer.SetSineTable(tableWords<512,128>());renderer.SetAfcCoeffs(afcCoefficients());renderer.SetOutputVolume(0x1000);}
    void begin(){renderer.SetOutputLeftBufferAddr(leftAddress);renderer.SetOutputRightBufferAddr(rightAddress);renderer.PrepareFrame();}
    void voice(JASDsp::TChannel& channel,unsigned index=0){
        assert(Native::encodeDspVoice(channel,table.data()+index*Native::dspVoiceBytes,Native::dspVoiceBytes));
        renderer.AddVoice(index);
        assert(Native::decodeDspVoiceState(table.data()+index*Native::dspVoiceBytes,Native::dspVoiceBytes,channel));
    }
    std::array<Native::s16,160> end(){
        renderer.FinalizeFrame();std::array<Native::s16,160> output{};
        for(unsigned i=0;i<80;++i){const Native::u16 a=(Native::u16(left[i*2])<<8)|left[i*2+1],b=(Native::u16(right[i*2])<<8)|right[i*2+1];std::memcpy(&output[i*2],&a,2);std::memcpy(&output[i*2+1],&b,2);}
        return output;
    }
};
static JASDsp::TChannel pcmVoice(unsigned length,unsigned base=0,unsigned format=3){
    JASDsp::TChannel voice{};
    JASWaveInfo info;info.mFormat=format;info.mLoopFlags=0;info.mSampleCount=length;
    info.mSampleLoopStart=0;info.mSampleLoopEnd=length;info.mpLastSample=info.mpPenultSample=0;
    voice.setWaveInfo(info,base,0);voice.playStart();voice.mPitch=0x1000;
    voice._10[0][0]=0xD00;voice._10[1][0]=0xD60;
    for(unsigned ch=0;ch<2;++ch)voice._10[ch][1]=voice._10[ch][2]=0x7FFF;
    return voice;
}
static void synthetic(){
    JASDsp::TChannel original{};auto* raw=reinterpret_cast<unsigned char*>(&original);
    for(unsigned i=0;i<sizeof(original);++i)raw[i]=unsigned(i*73+19);
    std::array<unsigned char,Native::dspVoiceBytes> wire{};
    assert(Native::encodeDspVoice(original,wire.data(),wire.size()));
    JASDsp::TChannel roundtrip;std::memset(&roundtrip,0xA5,sizeof(roundtrip));
    assert(Native::decodeDspVoiceState(wire.data(),wire.size(),roundtrip));
    assert(!std::memcmp(&roundtrip,&original,0x100));
    for(unsigned i=0x100;i<sizeof(roundtrip);++i)assert(reinterpret_cast<unsigned char*>(&roundtrip)[i]==0xA5);
    assert(!Native::encodeDspVoice(original,nullptr,wire.size()));
    assert(!Native::encodeDspVoice(original,wire.data(),wire.size()-1));
    assert(!Native::decodeDspVoiceState(wire.data(),255,roundtrip));

    RendererFixture test;std::vector<unsigned char> samples(400);
    for(unsigned i=0;i<200;++i){samples[i*2]=0x27;samples[i*2+1]=0x10;} // +10000 BE PCM16.
    test.renderer.SetARAMBaseAddr(test.addresses.map(samples.data(),samples.size()));
    auto voice=pcmVoice(200);
    test.begin();test.voice(voice);auto output=test.end();
    // Actual original phase-zero coefficients sum to32779. At unity pitch the
    // settled resampled value is10003; original1.16 voice mixing gives5001.
    for(unsigned i=4;i<80;++i)assert(output[i*2]==5001&&output[i*2+1]==5001);
    assert(voice._70==160&&voice._74==120&&voice._8==0&&!voice.mIsFinished);
    test.begin();test.voice(voice);test.end();assert(voice._74==40);
    test.begin();test.voice(voice);test.end();assert(voice._74==0&&voice.mIsFinished);
    test.begin();test.voice(voice);output=test.end();for(auto sample:output)assert(sample==0);

    voice=pcmVoice(200);voice.mPitch=0x800;test.begin();test.voice(voice);test.end();
    assert(voice._70==80&&voice._74==160);
    voice=pcmVoice(200);voice.mPitch=0x1800;test.begin();test.voice(voice);test.end();
    assert(voice._70==240&&voice._74==80);

    voice=pcmVoice(200);voice._102=1;voice._110=40;
    for(unsigned frame=0;frame<3;++frame){test.begin();test.voice(voice);test.end();}
    assert(!voice.mIsFinished&&voice._74==120&&voice._70==160);
    const auto position=voice._70,remaining=voice._74;voice.mPauseFlag=1;
    test.begin();test.voice(voice);test.end();assert(voice._70==position&&voice._74==remaining);
    voice.mPauseFlag=0;voice.mForcedStop=1;
    unsigned stoppedAt=0;
    for(unsigned frame=1;frame<=32;++frame){test.begin();test.voice(voice);test.end();if(voice.mIsFinished){stoppedAt=frame;break;}}
    assert(stoppedAt>1&&stoppedAt<=16);
    bool rejected=false;try{test.renderer.AddVoice(64);}catch(const std::out_of_range&){rejected=true;}assert(rejected);
    rejected=false;try{test.memory.GetSamplePointer(0xFFFFFFFFu,2,1);}catch(const std::out_of_range&){rejected=true;}assert(rejected);
    test.renderer.SetOutputRightBufferAddr(1);test.begin(); // begin restores valid addresses.
    test.renderer.SetOutputRightBufferAddr(1);const auto before=test.left;
    rejected=false;try{test.renderer.FinalizeFrame();}catch(const std::out_of_range&){rejected=true;}assert(rejected&&test.left==before);
    RendererFixture shortInput;std::array<unsigned char,16> shortSamples{};
    shortInput.renderer.SetARAMBaseAddr(shortInput.addresses.map(shortSamples.data(),shortSamples.size()));
    voice=pcmVoice(200);shortInput.begin();rejected=false;
    try{shortInput.voice(voice);}catch(const std::out_of_range&){rejected=true;}assert(rejected&&!voice.mIsFinished);
    RendererFixture badReverb;badReverb.reverb[1]=1;rejected=false;
    try{badReverb.begin();}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
    std::printf("PASS original renderer PCM16 samples/state, exact settled sample5001, EOF, loop, pause, gradual release in%u subframes, wire/writeback and bounds\n",stoppedAt);
}
static void compressed(){
    assert(afcCoefficients()[2]==0x800&&afcCoefficients()[3]==0);
    for(unsigned format:{0u,1u})for(unsigned predictor:{0u,1u}){
        const unsigned blockBytes=format==0?9:5;
        std::vector<unsigned char> compressed(blockBytes*10),pcm(320);
        int previous=0;
        for(unsigned i=0;i<160;++i){
            const unsigned block=i/16,index=i%16;
            compressed[block*blockBytes]=0x80|predictor; // exponent8.
            const int value=format==0?int(index)-8:int(index%4)-2;
            const unsigned bits=unsigned(value)&(format==0?15:3);
            const unsigned byte=1+(format==0?index/2:index/4),shift=format==0?(index%2?0:4):(6-(index%4)*2);
            compressed[block*blockBytes+byte]|=bits<<shift;
            const int raw=std::clamp(value*(format==0?256:1024)+(predictor?previous:0),-32768,32767);
            previous=raw;const auto word=Native::u16(raw);pcm[i*2]=word>>8;pcm[i*2+1]=word;
        }
        RendererFixture encoded,reference;
        encoded.renderer.SetARAMBaseAddr(encoded.addresses.map(compressed.data(),compressed.size()));
        reference.renderer.SetARAMBaseAddr(reference.addresses.map(pcm.data(),pcm.size()));
        auto a=pcmVoice(160,0,format),b=pcmVoice(160);
        for(unsigned frame=0;frame<4;++frame){encoded.begin();reference.begin();encoded.voice(a);reference.voice(b);assert(encoded.end()==reference.end());}
        assert(a.mIsFinished&&b.mIsFinished);
    }
    std::puts("PASS original AFC HQ/LQ decoding and predictor history match independent PCM references through resampling/mixing and real voice completion");
}
static void put16(std::ofstream& out,unsigned v){out.put(char(v));out.put(char(v>>8));}
static void put32(std::ofstream& out,unsigned v){put16(out,v);put16(out,v>>16);}
static void retail(const char* path,const char* destination){
    WiiAssets disc(std::filesystem::u8path(path));
    for(const auto& file:disc.files){
        if(file.path.rfind("/AudioRes/Stream/",0))continue;
        const auto bytes=disc.read(file.offset,64);JASAramStream::Header header{};
        assert(Native::decodeJasStreamHeader(bytes.data(),bytes.size(),header));
        if(header.format!=1||header.channels!=2)continue;
        const auto blockBytes=32+std::size_t(header.block_size)*header.channels;
        auto block=disc.read(file.offset+64,blockBytes);JASAramStream::BlockHeader decoded{};
        assert(Native::decodeJasStreamBlock(block.data(),block.size(),2,header.block_size,decoded));
        std::vector<Native::s16> reference;
        assert(Native::decodeJasStreamPcm16(block.data(),block.size(),2,header.block_size,decoded._4/2,reference));
        if(std::all_of(reference.begin(),reference.end(),[](auto v){return v==0;}))continue;
        RendererFixture test;const auto samples=decoded._4/2;
        test.renderer.SetARAMBaseAddr(test.addresses.map(block.data()+32,decoded._4*2));
        auto left=pcmVoice(samples),right=pcmVoice(samples,decoded._4);
        left._10[1][0]=0;right._10[0][0]=0;
        std::vector<Native::s16> output;
        for(unsigned frame=0;frame<(samples+79)/80+1;++frame){test.begin();test.voice(left,0);test.voice(right,1);auto next=test.end();output.insert(output.end(),next.begin(),next.end());}
        assert(left.mIsFinished&&right.mIsFinished);
        assert(std::any_of(output.begin(),output.end(),[](auto v){return v!=0;}));
        std::ofstream out(destination,std::ios::binary);assert(out);
        const auto dataBytes=unsigned(output.size()*2);out.write("RIFF",4);put32(out,36+dataBytes);out.write("WAVEfmt ",8);put32(out,16);put16(out,1);put16(out,2);put32(out,header._10);put32(out,header._10*4);put16(out,4);put16(out,16);out.write("data",4);put32(out,dataBytes);for(auto value:output)put16(out,Native::u16(value));
        std::printf("PASS retail stereo PCM16 %s: %u source frames, %zu rendered frames, actual EOF state and nonzero stereo WAV\n",file.path.c_str(),samples,output.size()/2);return;
    }
    throw std::runtime_error("No non-silent stereo PCM16 retail stream first block found");
}
int main(int argc,char**argv){try{synthetic();compressed();if(argc==3)retail(argv[1],argv[2]);else assert(argc==1);}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}}
