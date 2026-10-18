#include <cstdint>
#include <JSystem/JKernel/JKRAramArchive.hpp>
#include <JSystem/JKernel/JKRAram.hpp>
#include <JSystem/JKernel/JKRDecomp.hpp>
#include <JSystem/JUtility/JUTException.hpp>
#define ALIGN_PREV(X,N) ((X)&~(decltype(X))((N)-1))
#define ALIGN_NEXT(X,N) ALIGN_PREV((X)+(N)-1,N)
u32 JKRAramArchive::fetchResource_subroutine(u32 srcAram, u32 srcLength, u8* dst, u32 dstLength, int compression) {
    u32 outLen;
    u32 srcSize = ALIGN_NEXT(srcLength, 0x20);
    u32 dstSize = ALIGN_PREV(dstLength, 0x20);

    switch (compression) {
    case COMPRESSION_NONE:
        if (srcSize > dstSize) {
            srcSize = dstSize;
        }
        JKRAramToMainRam(srcAram, dst, srcSize, EXPAND_SWITCH_UNKNOWN0, dstSize, NULL, -1, &outLen);
        return outLen;
    case COMPRESSION_YAY0:
    case COMPRESSION_YAZ0:
        JKRAramToMainRam(srcAram, dst, srcSize, EXPAND_SWITCH_UNKNOWN1, dstSize, NULL, -1, &outLen);
        return outLen;
    default:
        JUTException::panic(__FILE__, 655, "??? bad sequence\n");
        return 0;
    }
}

u32 JKRAramArchive::fetchResource_subroutine(u32 entryNum, u32 length, JKRHeap* pHeap, int compression, u8** out) {
    u32 alignedLen = ALIGN_NEXT(length, 0x20);

    u8* buffer;
    switch (compression) {
    case COMPRESSION_NONE: {
        buffer = static_cast< u8* >(JKRAllocFromHeap(pHeap, alignedLen, 0x20));
        JKRAramToMainRam(entryNum, buffer, alignedLen, EXPAND_SWITCH_UNKNOWN0, alignedLen, NULL, -1, NULL);
        *out = buffer;
        return length;
    }
    case COMPRESSION_YAY0:
    case COMPRESSION_YAZ0: {
        u8 headerBuf[0x40];
        u8* alignHeader = reinterpret_cast< u8* >(ALIGN_NEXT(reinterpret_cast< std::uintptr_t >(&headerBuf[0]), sizeof(RarcHeader)));
        JKRAramToMainRam(entryNum, alignHeader, sizeof(RarcHeader), EXPAND_SWITCH_UNKNOWN0, 0, NULL, -1, NULL);
        u32 decompressedLen = ALIGN_NEXT(JKRDecompExpandSize(alignHeader), sizeof(RarcHeader));
        buffer = static_cast< u8* >(JKRAllocFromHeap(pHeap, decompressedLen, sizeof(RarcHeader)));
        u32 readLen;
        JKRAramToMainRam(entryNum, buffer, alignedLen, EXPAND_SWITCH_UNKNOWN1, decompressedLen, pHeap, -1, &readLen);
        *out = buffer;
        return readLen;
    }
    default:
        JUTException::panic(__FILE__, 713, "??? bad sequence\n");
        return 0;
    }
}

