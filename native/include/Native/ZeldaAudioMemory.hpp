#pragma once
#include <Native/DspMemoryView.hpp>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <vector>
namespace Native {
using u8=std::uint8_t;using s8=std::int8_t;
using u16=std::uint16_t;using s16=std::int16_t;
using u32=std::uint32_t;using s32=std::int32_t;
using u64=std::uint64_t;using s64=std::int64_t;

// Canonical DSP wire memory: big-endian words/samples, with bounded registered
// ranges. Native JAS host structures require an explicit serialization adapter;
// they must never be passed through this view as if they were wire-format data.
class ZeldaAudioMemory {
    const DspAddressSpace& addresses;
    DspMemoryView memory;
    using Reader=bool(*)(void*,u32,void*,std::size_t);
    Reader sampleReader_=nullptr;
    void* sampleContext_=nullptr;
    u32 sampleBase_=0;
    mutable std::vector<u16> sampleCopy_;
public:
    using SampleReader=Reader;
    explicit ZeldaAudioMemory(const DspAddressSpace& space):addresses(space),memory(space){}
    // Native ARAM uses offsets into its own backing, rather than exposing a
    // host pointer. Each read returns an aligned copy stable until the next
    // sample read on this single-owner memory view. Other DSP memory stays mapped.
    void setSampleReader(u32 base,SampleReader reader,void* context)noexcept{
        sampleBase_=base;sampleReader_=reader;sampleContext_=context;
    }
    void CheckRange(u32 address,std::size_t bytes)const{
        if(!addresses.resolve(address,bytes))throw std::out_of_range("Zelda audio memory range is not registered");
    }
    template<class T>void CopyFromEmuSwapped(T* output,u32 address,std::size_t bytes)const{
        static_assert(sizeof(T)==2);
        if(!memory.copyBigEndianWordsToHost(address,output,bytes))throw std::out_of_range("Zelda audio input word range invalid");
    }
    template<class T>void CopyToEmuSwapped(u32 address,const T* input,std::size_t bytes)const{
        static_assert(sizeof(T)==2);
        if(!input||bytes%2)throw std::invalid_argument("Zelda audio output words invalid");
        CheckRange(address,bytes);
        for(std::size_t i=0;i<bytes/2;++i){
            u16 bits;std::memcpy(&bits,input+i,2);
            if(!memory.writeBigEndian16(address+static_cast<u32>(i*2),bits))throw std::out_of_range("Zelda audio output word write failed");
        }
    }
    u8* GetSamplePointer(u32 base,u32 offset,std::size_t bytes)const{
        if(sampleReader_&&base==sampleBase_){
            if(bytes>std::numeric_limits<u32>::max()-offset)throw std::out_of_range("Native ARAM sample address overflow");
            sampleCopy_.resize(bytes/2+1);
            if(!sampleReader_(sampleContext_,offset,sampleCopy_.data(),bytes))throw std::out_of_range("Native ARAM sample read rejected");
            return reinterpret_cast<u8*>(sampleCopy_.data());
        }
        const u64 address=u64(base)+offset;
        if(address>std::numeric_limits<u32>::max())throw std::out_of_range("Zelda audio sample address overflow");
        auto* data=addresses.resolve(static_cast<u32>(address),bytes);
        if(!data)throw std::out_of_range("Zelda audio sample range invalid");
        return static_cast<u8*>(data);
    }
    template<class T>static T FromBigEndian(T value){
        static_assert(sizeof(T)==1||sizeof(T)==2);
        if constexpr(sizeof(T)==2){u16 bits;std::memcpy(&bits,&value,2);bits=u16((bits>>8)|(bits<<8));std::memcpy(&value,&bits,2);}
        return value;
    }
};
}
