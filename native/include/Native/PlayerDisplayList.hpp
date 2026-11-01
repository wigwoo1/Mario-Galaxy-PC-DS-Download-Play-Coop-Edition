#pragma once
#include <Native/HeapGpuRange.hpp>
namespace Native {
// GD commands already have GX byte order. Keep a raw snapshot, not native-word conversion.
inline u8* allocatePlayerDisplayList(std::size_t capacity){
    if(!capacity)throw std::invalid_argument("Empty player display list");
    if(capacity>std::numeric_limits<u32>::max())throw std::bad_array_new_length();
    auto* heap=JKRHeap::sCurrentHeap;
    if(!heap)throw std::logic_error("Player display lists require a current heap");
    auto* bytes=new(heap,32) u8[capacity]{};
    try{bindHeapGpuRange(bytes,capacity);}
    catch(...){JKRHeap::free(bytes,heap);throw;}
    return bytes;
}
inline void releasePlayerDisplayList(u8* bytes){
    if(!bytes)return;
    auto* heap=JKRHeap::findFromRoot(bytes);
    releaseHeapGpuRange(bytes);
    JKRHeap::free(bytes,heap);
}
}