#pragma once
#include <revolution/types.h>
inline u32 __cntlzw(u32 value){
    if(!value)return 32;
    u32 count=0;for(u32 bit=0x80000000;!(value&bit);bit>>=1)++count;
    return count;
}
namespace Native {
// Required graphics backend provider: return a 32-byte-aligned emulated address
// whose storage resolves to this host resource. Never truncate a host pointer.
u32 gpuResourceAddress(const void* resource);
}
