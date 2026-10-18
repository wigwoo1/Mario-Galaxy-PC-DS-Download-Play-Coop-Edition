#include <JSystem/JKernel/JKRAram.hpp>
#include <JSystem/JKernel/JKRAramStream.hpp>
#include <JSystem/JKernel/JKRDecomp.hpp>
#include <Native/HeapPlacement.hpp>

JKRAram* JKRAram::create(u32 audioSize, u32 graphSize, s32 streamPriority,
                         s32 decompPriority, s32 piecePriority) {
    if (!sAramObject) {
        sAramObject = new (JKRGetSystemHeap(), 0)
            JKRAram(audioSize, graphSize, piecePriority);
    }

    JKRCreateAramStreamManager(streamPriority);
    JKRCreateDecompManager(decompPriority);
    sAramObject->resume();
    return sAramObject;
}
