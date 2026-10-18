#pragma once
#include <stdint.h>
inline uint32_t nativeUnitHeapBitmapWord(const void* address){
 const auto* bytes=static_cast<const uint8_t*>(address);
 return uint32_t(bytes[0])<<24 | uint32_t(bytes[1])<<16 | uint32_t(bytes[2])<<8 | uint32_t(bytes[3]);
}
inline int32_t nativeUnitHeapAddressIndex(const void* address,const void* base,uint32_t unitSize,uint32_t unitCount){
 if(!address || !base || !unitSize)return -1;
 const uintptr_t position=reinterpret_cast<uintptr_t>(address),start=reinterpret_cast<uintptr_t>(base);
 if(position<start)return -1;
 const uintptr_t offset=position-start;
 if(offset%unitSize)return -1;
 const uintptr_t index=offset/unitSize;
 return index<unitCount && index<=INT32_MAX?int32_t(index):-1;
}
struct NativeUnitHeapFreeRun {uint32_t start=0,count=0;};
inline NativeUnitHeapFreeRun nativeUnitHeapLongestFreeRun(const uint8_t* bitmap,uint32_t unitCount){
 NativeUnitHeapFreeRun best;uint32_t start=0,count=0;
 for(uint32_t index=0;index<unitCount;++index){if(bitmap[index/8]&(0x80u>>(index&7))){count=0;}else{if(!count)start=index;++count;if(count>best.count)best={start,count};}}
 return best;
}
