// SPDX-License-Identifier: GPL-2.0-or-later
#include <Native/DspNativeAudioBridge.hpp>
#include <Native/DspVoiceWire.hpp>
#include <cstring>
#include <stdexcept>
namespace Native {
namespace {
void put16(u8* p,u16 value){p[0]=value>>8;p[1]=value;}
void put32(u8* p,u32 value){put16(p,value>>16);put16(p+2,value);}
void nativeToWire(const void* from,u8* to,std::size_t bytes){
    auto* input=static_cast<const u8*>(from);
    for(std::size_t i=0;i<bytes;i+=2){u16 word;std::memcpy(&word,input+i,2);put16(to+i,word);}
}
void wireToNative(const u8* from,void* to,std::size_t bytes){
    auto* output=static_cast<u8*>(to);
    for(std::size_t i=0;i<bytes;i+=2){u16 word=(u16(from[i])<<8)|from[i+1];std::memcpy(output+i,&word,2);}
}
DspNativeAudioBridge::Reply rejected(DspNativeAudioBridge::Result result){
    DspNativeAudioBridge::Reply reply;reply.result=result;return reply;
}
}
DspNativeAudioBridge::DspNativeAudioBridge(const DspAddressSpace& host,ZeldaAudioMemory::SampleReader aram,void* context):host_(host){
    if(!aram)throw std::invalid_argument("Native DSP requires an ARAM sample reader");
    wireTables_={wire_.map(voices_.data(),voices_.size()),wire_.map(coefficients_.data(),coefficients_.size()),
                 wire_.map(afc_.data(),afc_.size()),wire_.map(effects_.data(),effects_.size())};
    wireLeft_=wire_.map(left_.data(),left_.size());wireRight_=wire_.map(right_.data(),right_.size());
    wireAram_=wire_.map(&aramTag_,1);
    for(auto address:wireTables_)if(!address)throw std::runtime_error("Native DSP wire table mapping failed");
    if(!wireLeft_||!wireRight_||!wireAram_)throw std::runtime_error("Native DSP wire buffer mapping failed");
    memory_.setSampleReader(wireAram_,aram,context);
}
void* DspNativeAudioBridge::hostSpan(u32 address,std::size_t bytes)const{
    auto* data=host_.resolve(address,bytes);
    if(!data)throw std::out_of_range("Native JAS buffer is not registered");return data;
}
void DspNativeAudioBridge::refreshEffects(){
    const auto* host=static_cast<const u8*>(hostSpan(hostTables_[3],4*sizeof(JASDsp::FxBuf)));
    for(unsigned line=0;line<4;++line){
        JASDsp::FxBuf fx;std::memcpy(&fx,host+line*sizeof(fx),sizeof(fx));
        auto& ring=rings_[line];u32 address=0;ring.host=0;ring.used=0;
        if(fx._0){
            const std::size_t bytes=std::size_t(fx._2)*0xA0;
            if(!bytes||!fx._4)throw std::out_of_range("Native JAS effects ring is empty");
            const u32 nativeAddress=host_.addressOf(fx._4,bytes);
            if(!nativeAddress)throw std::out_of_range("Native JAS effects ring is not registered");
            hostSpan(nativeAddress,bytes);
            if(ring.bytes.size()!=bytes){
                if(ring.wire)wire_.unmap(ring.wire);
                ring.bytes.resize(bytes);ring.wire=wire_.map(ring.bytes.data(),bytes);
                if(!ring.wire)throw std::out_of_range("Native JAS effects wire mapping failed");
            }
            nativeToWire(fx._4,ring.bytes.data(),bytes);
            ring.host=nativeAddress;ring.used=bytes;address=ring.wire;
        }
        auto* output=effects_.data()+line*0x20;
        put16(output,fx._0);put16(output+2,fx._2);put32(output+4,address);
        put16(output+8,fx._8);put16(output+10,u16(fx._A));
        put16(output+12,fx._C);put16(output+14,u16(fx._E));
        for(unsigned i=0;i<8;++i)put16(output+16+i*2,fx._10[i]);
        // Preserve IDs verbatim, including unrecognized SEND_TABLE entries.
        // The strict renderer must report unsupported buses, never drop them.
    }
}
void DspNativeAudioBridge::commitOutput(unsigned subframe){
    const unsigned offset=subframe*0xA0;
    auto* left=hostSpan(hostOutput_[0]+offset,0xA0);
    auto* right=hostSpan(hostOutput_[1]+offset,0xA0);
    // Resolve every destination before writing either plane or effects state.
    for(const auto& ring:rings_)if(ring.used)hostSpan(ring.host,ring.used);
    wireToNative(left_.data()+offset,left,0xA0);wireToNative(right_.data()+offset,right,0xA0);
    for(const auto& ring:rings_)if(ring.used)wireToNative(ring.bytes.data(),hostSpan(ring.host,ring.used),ring.used);
}
DspNativeAudioBridge::Reply DspNativeAudioBridge::processBatch(const u32* words,std::size_t count){
    if(failed_)return rejected(Result::WrongOrder);
    DspAudioCommandHle::Command command;
    const auto valid=DspAudioCommandHle::decodeBatch(words,count,command);
    if(valid==DspAudioCommandHle::Validation::Malformed)return rejected(Result::Malformed);
    if(valid==DspAudioCommandHle::Validation::Unsupported)return rejected(Result::Unsupported);
    using State=DspAudioFrameExecutor::State;using Kind=DspAudioCommandHle::Kind;
    // Reject invalid ordering before copying or changing host/wire records.
    if(command.kind==Kind::ChannelGroup){
        if(executor_.state()!=State::Rendering||executor_.nextGroup()!=command.group)return rejected(Result::WrongOrder);
    }else if(executor_.state()!=State::Idle)return rejected(Result::WrongOrder);
    try{
        if(command.kind==Kind::Varam){
            // Native sample addresses are offsets in the bound ARAM transport.
            // The original packet's Wii relocation/32-bit value is not a host
            // pointer. The caller supplies the actual bounded ARAM reader.
            const u32 packet[]={0x8E000000,wireAram_};return executor_.processBatch(packet,2);
        }
        if(command.kind==Kind::Tables){
            const auto& args=command.arguments;
            hostSpan(args[0],64*sizeof(JASDsp::TChannel));
            const auto* coefficients=static_cast<const u8*>(hostSpan(args[1],0x500));
            auto* afc=hostSpan(args[2],0x40);hostSpan(args[3],4*sizeof(JASDsp::FxBuf));
            for(unsigned i=0;i<320;++i){u32 word;std::memcpy(&word,coefficients+i*4,4);put32(coefficients_.data()+i*4,word);}
            std::memcpy(afc_.data(),afc,afc_.size()); // Original AFC table is already a BE byte array.
            hostTables_=args;
            const u32 packet[]={0x81000040,wireTables_[0],wireTables_[1],wireTables_[2],wireTables_[3]};
            return executor_.processBatch(packet,5);
        }
        if(command.kind==Kind::Frame){
            if(!command.subframes)return rejected(Result::Malformed);
            const auto bytes=std::size_t(command.subframes)*0xA0;
            hostSpan(command.arguments[0],bytes);hostSpan(command.arguments[1],bytes);
            const u32 packet[]={words[0],wireLeft_,wireRight_,words[3],words[4]};
            auto reply=executor_.processBatch(packet,5);
            if(reply.result==Result::Applied)hostOutput_={command.arguments[0],command.arguments[1]};
            return reply;
        }
        const unsigned group=command.group;
        auto* host=static_cast<u8*>(hostSpan(hostTables_[0],64*sizeof(JASDsp::TChannel)));
        if(!group)refreshEffects();
        for(unsigned i=group*16;i<(group+1)*16;++i){
            JASDsp::TChannel voice;std::memcpy(&voice,host+i*sizeof(voice),sizeof(voice));
            encodeDspVoice(voice,voices_.data()+i*0x180,0x180);
        }
        auto reply=executor_.processBatch(words,count);
        if(reply.result==Result::RenderFailed){failed_=true;return reply;}
        for(unsigned i=group*16;i<(group+1)*16;++i){
            JASDsp::TChannel voice{};decodeDspVoiceState(voices_.data()+i*0x180,0x180,voice);
            std::memcpy(host+i*sizeof(voice),&voice,0x100);
        }
        if(reply.result==Result::SubframeRendered)commitOutput(executor_.completedSubframes()-1);
        return reply;
    }catch(const std::out_of_range&){
        if(command.kind==Kind::ChannelGroup)failed_=true;
        return rejected(Result::InvalidMemory);
    }catch(...){failed_=true;return rejected(Result::RenderFailed);}
}
}
