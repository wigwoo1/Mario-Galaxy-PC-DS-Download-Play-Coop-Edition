#include <Native/InputWorkspace.hpp>
#include <Native/HeapPlacement.hpp>
#include <stdint.h>
#include "Game/System/HeapMemoryWatcher.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <revolution/os.h>
#include <revolution/wpad.h>
#if defined(_WIN32)
#include <windows.h>
#endif

#if defined(GALAXY_BOOT_TRACE)
extern "C" void galaxyNativeBootTrace(const char* step);
#define GALAXY_HEAP_TRACE(step) galaxyNativeBootTrace(step)
#else
#define GALAXY_HEAP_TRACE(step) ((void)0)
#endif

JKRExpHeap* HeapMemoryWatcher::sRootHeapGDDR3;

namespace {
    JKRExpHeap* createExpHeap(u32 size, JKRHeap* pHeap, bool a3) NO_INLINE {
        JKRExpHeap* heap;

        if (a3) {
            void* data = new (pHeap, -4) u8[size];
            heap = JKRExpHeap::create(data, size, pHeap, true);
        } else {
            heap = JKRExpHeap::create(size, pHeap, true);
        }

        if (MR::isEqualCurrentHeap(heap)) {
            JKRHeap::sRootHeap->becomeCurrentHeap();
        }

        return heap;
    }

    JKRSolidHeap* createSolidHeap(u32 size, JKRHeap* pHeap) {
        JKRSolidHeap* heap = JKRSolidHeap::create(size, pHeap, true);

        if (MR::isEqualCurrentHeap(heap)) {
            JKRHeap::sRootHeap->becomeCurrentHeap();
        }

        return heap;
    }

    void destroyHeapAndSetNULL(JKRHeap** pHeap) {
        if (*pHeap != nullptr) {
            JKRHeap::destroy(*pHeap);
            *pHeap = nullptr;
        }
    }
};  // namespace

JKRHeap* HeapMemoryWatcher::getHeapNapa(const JKRHeap* pHeap) {
    if (pHeap == mStationedHeapNapa || pHeap == mStationedHeapGDDR) {
        return mStationedHeapNapa;
    }

    if (pHeap == mSceneHeapNapa || pHeap == mSceneHeapGDDR) {
        return mSceneHeapNapa;
    }

    return nullptr;
}

JKRHeap* HeapMemoryWatcher::getHeapGDDR3(const JKRHeap* pHeap) {
    if (pHeap == mStationedHeapNapa || pHeap == mStationedHeapGDDR) {
        return mStationedHeapGDDR;
    }

    if (pHeap == mSceneHeapNapa || pHeap == mSceneHeapGDDR) {
        return mSceneHeapGDDR;
    }

    return nullptr;
}

void HeapMemoryWatcher::createFileCacheHeapOnGameHeap(u32 size) {
    mFileCacheHeap = ::createSolidHeap(size, mGameHeapGDDR);
}

void HeapMemoryWatcher::createSceneHeapOnGameHeap() {
    mSceneHeapNapa = ::createSolidHeap(-1, mGameHeapNapa);
    mSceneHeapGDDR = ::createSolidHeap(-1, mGameHeapGDDR);
}

void HeapMemoryWatcher::adjustStationedHeaps() {
    MR::adjustHeapSize(mStationedHeapNapa, 0);
    MR::adjustHeapSize(mStationedHeapGDDR, 0);
}

void HeapMemoryWatcher::setCurrentHeapToStationedHeap() {
    MR::becomeCurrentHeap(mStationedHeapNapa);
}

void HeapMemoryWatcher::setCurrentHeapToGameHeap() {
    MR::becomeCurrentHeap(JKRHeap::sSystemHeap);
}

void HeapMemoryWatcher::setCurrentHeapToSceneHeap() {
    MR::becomeCurrentHeap(mSceneHeapNapa);
}

void HeapMemoryWatcher::destroySceneHeap() {
    ::destroyHeapAndSetNULL(reinterpret_cast< JKRHeap** >(&mSceneHeapNapa));
    ::destroyHeapAndSetNULL(reinterpret_cast< JKRHeap** >(&mSceneHeapGDDR));
}

void HeapMemoryWatcher::destroyGameHeap() {
    if (mSceneHeapNapa != nullptr) {
        ::destroyHeapAndSetNULL(reinterpret_cast< JKRHeap** >(&mSceneHeapNapa));
    }

    if (mSceneHeapGDDR != nullptr) {
        ::destroyHeapAndSetNULL(reinterpret_cast< JKRHeap** >(&mSceneHeapGDDR));
    }

    if (mFileCacheHeap != nullptr) {
        ::destroyHeapAndSetNULL(reinterpret_cast< JKRHeap** >(&mFileCacheHeap));
    }

    ::destroyHeapAndSetNULL(reinterpret_cast< JKRHeap** >(&mGameHeapNapa));
    ::destroyHeapAndSetNULL(reinterpret_cast< JKRHeap** >(&mGameHeapGDDR));
    createGameHeap();
}

void HeapMemoryWatcher::createRootHeap() {
    GALAXY_HEAP_TRACE("heap: createRootHeap begin");
    JKRExpHeap* pHeap;
    void* newHi;
    uintptr_t arenaHi, arenaLo;

    if (JKRHeap::sRootHeap || sRootHeapGDDR3) OSPanic(__FILE__, __LINE__, "Root heaps already initialized");
    arenaLo = reinterpret_cast< uintptr_t >(OSGetMEM2ArenaLo());
    arenaHi = reinterpret_cast< uintptr_t >(OSGetMEM2ArenaHi());
    if (!arenaLo || arenaHi <= arenaLo || arenaHi-arenaLo < 0xE00000 + sizeof(JKRExpHeap) + sizeof(JKRExpHeap::CMemBlock) + 32 || arenaHi-arenaLo > UINT32_MAX)
        OSPanic(__FILE__, __LINE__, "Invalid secondary heap arena");
    if (!JKRExpHeap::createRoot(1, true)) OSPanic(__FILE__, __LINE__, "Missing primary heap arena");
    GALAXY_HEAP_TRACE("heap: primary root created");
    newHi = reinterpret_cast< void* >(arenaLo + 0xE00000);
    OSSetMEM2ArenaHi(newHi);
    JKRHeap::setAltAramStartAdr(arenaLo);
    pHeap = JKRExpHeap::create(newHi, arenaHi - reinterpret_cast< uintptr_t >(newHi), JKRHeap::sRootHeap, true);
    GALAXY_HEAP_TRACE("heap: secondary root created");

    if (MR::isEqualCurrentHeap(pHeap)) {
        JKRHeap::sRootHeap->becomeCurrentHeap();
    }

    HeapMemoryWatcher::sRootHeapGDDR3 = pHeap;
}

void HeapMemoryWatcher::createHeaps() {
    GALAXY_HEAP_TRACE("heap: createHeaps begin");
    MR::CurrentHeapRestorer heapRestorer = MR::CurrentHeapRestorer(JKRHeap::sRootHeap);
    ::createExpHeap(0x40000, JKRHeap::sRootHeap, false)->becomeSystemHeap();
    GALAXY_HEAP_TRACE("heap: system heap created");
    // The native audio runtime needs space for the retail three 0xDAC0-byte
    // sequence buffers after constructing its archive pools and managers.
    mAudSystemHeap = ::createSolidHeap(0x200000, JKRHeap::sRootHeap);
    GALAXY_HEAP_TRACE("heap: audio heap created");
    // Stationed JPA resources retain native pointers and callback tables for
    // every retail effect. Scale this Wii word-sized budget for host pointers.
    constexpr u32 stationedBytes = 0x900000 * (sizeof(void*) / sizeof(u32));
    mStationedHeapNapa = ::createExpHeap(stationedBytes, JKRHeap::sRootHeap, false);
    GALAXY_HEAP_TRACE("heap: stationed MEM1 heap created");
    JKRHeap* pRootHeapGDDR = HeapMemoryWatcher::sRootHeapGDDR3;
    u32 wpadHeapSize = OSRoundUp32B(nativeInputWorkspaceBytes()) + 0xD0;
    mWPadHeap = ::createExpHeap(wpadHeapSize, pRootHeapGDDR, false);
    GALAXY_HEAP_TRACE("heap: controller heap created");
    mHomeButtonLayoutHeap = ::createExpHeap(0x80000, HeapMemoryWatcher::sRootHeapGDDR3, false);
    GALAXY_HEAP_TRACE("heap: home-button heap created");
    mStationedHeapGDDR = ::createExpHeap(0x2000000, HeapMemoryWatcher::sRootHeapGDDR3, false);
    GALAXY_HEAP_TRACE("heap: stationed MEM2 heap created");
    createGameHeap();
    GALAXY_HEAP_TRACE("heap: game heaps created");
}

void HeapMemoryWatcher::createGameHeap() {
    mGameHeapNapa = ::createExpHeap(-1, JKRHeap::sRootHeap, false);
    mGameHeapGDDR = ::createExpHeap(-1, HeapMemoryWatcher::sRootHeapGDDR3, false);
}

HeapMemoryWatcher::HeapMemoryWatcher()
    : mStationedHeapNapa(nullptr), mStationedHeapGDDR(nullptr), mGameHeapNapa(nullptr), mGameHeapGDDR(nullptr), mFileCacheHeap(nullptr),
      mSceneHeapNapa(nullptr), mSceneHeapGDDR(nullptr), mWPadHeap(nullptr), mHomeButtonLayoutHeap(nullptr), mAudSystemHeap(nullptr) {
    JKRHeap::setErrorHandler(HeapMemoryWatcher::memoryErrorCallback);
    GALAXY_HEAP_TRACE("heap: HeapMemoryWatcher constructor");
    createHeaps();
}

void HeapMemoryWatcher::memoryErrorCallback(void* heap, u32 size, int alignment) {
    auto* owner = static_cast<JKRHeap*>(heap);
    const u32 type = owner ? owner->getHeapType() : 0xFFFFFFFFu;
    const s32 totalFree = owner ? owner->getTotalFreeSize() : -1;
    const u32 maxAllocatable = owner ? owner->getMaxAllocatableSize(alignment) : 0;
    const void* start = owner ? owner->getStart() : nullptr;
    const void* end = owner ? owner->getEnd() : nullptr;
    void* frames[8]{};
#if defined(_WIN32)
    const USHORT frameCount = CaptureStackBackTrace(0, 8, frames, nullptr);
#else
    const USHORT frameCount = 0;
#endif
    OSPanic(__FILE__, 537,
            "heap=%p type=%u range=[%p,%p) free=%d maxAlloc=0x%08X requested=0x%08X alignment=%d frames=%u [%p,%p,%p,%p,%p,%p,%p,%p]",
            heap, type, start, end, totalFree, maxAllocatable, size, alignment, frameCount,
            frames[0], frames[1], frames[2], frames[3], frames[4], frames[5], frames[6], frames[7]);
}

void HeapMemoryWatcher::checkRestMemory() {
}
