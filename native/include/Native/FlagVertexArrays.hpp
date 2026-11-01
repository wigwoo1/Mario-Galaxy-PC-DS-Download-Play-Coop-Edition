#pragma once
#include <Native/HeapGpuRange.hpp>
#include <Game/Util/Color.hpp>
#include <revolution/os/OSCache.h>
namespace Native {
inline std::size_t flagVertexCount(s32 rows,s32 columns){
    if(rows<2||columns<1)throw std::invalid_argument("Invalid flag grid");
    const auto count=std::uint64_t(rows)*(std::uint64_t(columns)+1);
    if(count>std::numeric_limits<u32>::max()/(2*sizeof(f32)))throw std::bad_array_new_length();
    return static_cast<std::size_t>(count);
}
template<class T> inline T* allocateFlagArray(std::size_t count,bool nativeWords){
    static_assert(sizeof(Color8)==4);
    if(!count||count>std::numeric_limits<u32>::max()/sizeof(T))throw std::bad_array_new_length();
    auto* heap=JKRHeap::sCurrentHeap;
    if(!heap)throw std::logic_error("Flag arrays require a current heap");
    auto* data=new(heap,32) T[count]{};
    try{if(nativeWords)bindHeapGpuNativeWords(data,count*sizeof(T));else bindHeapGpuRange(data,count*sizeof(T));}
    catch(...){JKRHeap::free(data,heap);throw;}
    return data;
}
inline Color8* allocateFlagColors(s32 rows,s32 columns){
    return allocateFlagArray<Color8>(flagVertexCount(rows,columns),false);
}
inline f32* allocateFlagTexCoords(s32 rows,s32 columns){
    return allocateFlagArray<f32>(flagVertexCount(rows,columns)*2,true);
}
inline void refreshFlagVertexArrays(Color8* colors,f32* texCoords,s32 rows,s32 columns){
    const auto count=flagVertexCount(rows,columns);
    DCStoreRange(colors,static_cast<u32>(count*sizeof(Color8)));
    DCStoreRange(texCoords,static_cast<u32>(count*2*sizeof(f32)));
}
}