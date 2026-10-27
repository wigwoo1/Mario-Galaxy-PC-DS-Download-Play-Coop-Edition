#pragma once
#include <Native/HeapGpuRange.hpp>
#include <revolution/mtx.h>
#include <limits>
#include <stdexcept>
namespace Native {
// Player view matrices are shared by main/shadow models and live with the scene heap.
inline Mtx* allocateExtraModelMatrices(std::size_t count){
    if(!count)throw std::invalid_argument("Empty extra model matrix allocation");
    if(count>std::numeric_limits<u32>::max()/sizeof(Mtx))throw std::bad_array_new_length();
    auto* heap=JKRHeap::sCurrentHeap;
    if(!heap)throw std::logic_error("Extra model matrices require a current heap");
    auto* data=new(heap,32) Mtx[count]{};
    try{bindHeapGpuNativeWords(data,count*sizeof(Mtx));}
    catch(...){JKRHeap::free(data,heap);throw;}
    return data;
}
}