#pragma once
#include <Native/GpuResources.hpp>
#include <Native/HeapPlacement.hpp>
#include <JSystem/JKernel/JKRDisposer.hpp>
#include <cstdint>
#include <limits>
#include <mutex>
#include <unordered_map>
#include <stdexcept>
namespace Native {
class HeapGpuRange;
inline std::mutex& heapGpuRangeMutex(){static std::mutex value;return value;}
inline std::unordered_map<const void*,HeapGpuRange*>& heapGpuRanges(){static std::unordered_map<const void*,HeapGpuRange*> value;return value;}
inline bool hasHeapGpuRange(const void* data){std::lock_guard<std::mutex> lock(heapGpuRangeMutex());return heapGpuRanges().count(data)!=0;}
// The heap owns the bytes. Unregister before that heap releases its storage.
class HeapGpuRange final:public JKRDisposer {
 void* base;
public:
 HeapGpuRange(void* data,std::size_t size):HeapGpuRange(data,size,false){}
 HeapGpuRange(void* data,std::size_t size,bool nativeWords):base(data){
  if(nativeWords)gpuResources().addNativeWords(data,size);else gpuResources().addWritable(data,size);
  try{std::lock_guard<std::mutex> lock(heapGpuRangeMutex());if(!heapGpuRanges().emplace(data,this).second)throw std::invalid_argument("Duplicate heap GPU range");}
  catch(...){gpuResources().remove(data);throw;}
 }
 ~HeapGpuRange() override {
  {std::lock_guard<std::mutex> lock(heapGpuRangeMutex());heapGpuRanges().erase(base);}
  gpuResources().remove(base);
 }
};
inline void bindHeapGpuRangeImpl(void* data,std::size_t size,bool nativeWords){
 const auto address=reinterpret_cast<std::uintptr_t>(data);
 if(!data||!size||address>std::numeric_limits<std::uintptr_t>::max()-(size-1))throw std::invalid_argument("Invalid heap GPU range");
 auto* heap=JKRHeap::findFromRoot(data);
 if(!heap||JKRHeap::findFromRoot(reinterpret_cast<void*>(address+size-1))!=heap)throw std::invalid_argument("GPU range is not contained in its owning heap");
 new(heap,0) HeapGpuRange(data,size,nativeWords);
}
inline void bindHeapGpuRange(void* data,std::size_t size){bindHeapGpuRangeImpl(data,size,false);}
inline void bindHeapGpuNativeWords(void* data,std::size_t size){bindHeapGpuRangeImpl(data,size,true);}
// Explicit owners release the disposer before freeing their buffers early.
inline bool releaseHeapGpuRange(const void* data){
 HeapGpuRange* owner=nullptr;
 {std::lock_guard<std::mutex> lock(heapGpuRangeMutex());auto found=heapGpuRanges().find(data);if(found==heapGpuRanges().end())return false;owner=found->second;}
 auto* heap=JKRHeap::findFromRoot(owner);owner->~HeapGpuRange();JKRHeap::free(owner,heap);return true;
}
}