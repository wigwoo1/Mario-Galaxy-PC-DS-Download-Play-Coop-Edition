#include <JSystem/JKernel/JKRHeap.hpp>
void JKRHeap::copyMemory(void* pDst, void* pSrc, u32 size) {
    u32 count = (size + 3) / 4;
    u32* dst_32 = (u32*)pDst;
    u32* src_32 = (u32*)pSrc;

    while (count > 0) {
        *dst_32 = *src_32;
        dst_32++;
        src_32++;
        count--;
    }
}