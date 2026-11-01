#pragma once
#include <JSystem/JAudio2/JASDspInterface.hpp>
#include <array>
#include <cstring>
#include <cstddef>
namespace Native {
// JAS writes native scalar fields. The DSP wire record stores big-endian words,
// and its 32-bit positions/addresses use high-word/low-word order.
inline constexpr std::size_t dspVoiceBytes=0x180;
inline constexpr std::array<std::size_t,8> dspVoiceWideOffsets={0x68,0x70,0x74,0x10C,0x110,0x114,0x118,0x11C};
static_assert(sizeof(JASDsp::TChannel)==dspVoiceBytes);
static_assert(offsetof(JASDsp::TChannel,_68)==0x68&&offsetof(JASDsp::TChannel,_11C)==0x11C);
inline bool encodeDspVoice(const JASDsp::TChannel& voice,void* destination,std::size_t bytes){
    if(!destination||bytes<dspVoiceBytes)return false;
    const auto* source=reinterpret_cast<const unsigned char*>(&voice);auto* output=static_cast<unsigned char*>(destination);
    for(std::size_t i=0;i<dspVoiceBytes;i+=2){u16 value;std::memcpy(&value,source+i,2);output[i]=value>>8;output[i+1]=value;}
    for(auto i:dspVoiceWideOffsets){u32 value;std::memcpy(&value,source+i,4);for(unsigned j=0;j<4;++j)output[i+j]=value>>(24-j*8);}
    return true;
}
inline bool decodeDspVoiceState(const void* data,std::size_t bytes,JASDsp::TChannel& voice){
    constexpr std::size_t writable=0x100;
    if(!data||bytes<writable)return false;
    const auto* source=static_cast<const unsigned char*>(data);auto* output=reinterpret_cast<unsigned char*>(&voice);
    for(std::size_t i=0;i<writable;i+=2){const u16 value=(u16(source[i])<<8)|source[i+1];std::memcpy(output+i,&value,2);}
    for(auto i:dspVoiceWideOffsets){if(i>=writable)continue;const u32 value=(u32(source[i])<<24)|(u32(source[i+1])<<16)|(u32(source[i+2])<<8)|source[i+3];std::memcpy(output+i,&value,4);}
    return true;
}
}
