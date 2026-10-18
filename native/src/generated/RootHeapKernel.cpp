#include <stdint.h>
#include <Game/System/HeapMemoryWatcher.hpp>
#include <Game/Util/MemoryUtil.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <revolution/os.h>
JKRExpHeap* HeapMemoryWatcher::sRootHeapGDDR3;
namespace MR { bool isEqualCurrentHeap(JKRHeap* heap){return JKRHeap::sCurrentHeap==heap;} }
void HeapMemoryWatcher::createRootHeap() {
    JKRExpHeap* pHeap;
    void* newHi;
    uintptr_t arenaHi, arenaLo;

    if (JKRHeap::sRootHeap || sRootHeapGDDR3) OSPanic(__FILE__, __LINE__, "Root heaps already initialized");
    arenaLo = reinterpret_cast< uintptr_t >(OSGetMEM2ArenaLo());
    arenaHi = reinterpret_cast< uintptr_t >(OSGetMEM2ArenaHi());
    if (!arenaLo || arenaHi <= arenaLo || arenaHi-arenaLo < 0xE00000 + sizeof(JKRExpHeap) + sizeof(JKRExpHeap::CMemBlock) + 32 || arenaHi-arenaLo > UINT32_MAX)
        OSPanic(__FILE__, __LINE__, "Invalid secondary heap arena");
    if (!JKRExpHeap::createRoot(1, true)) OSPanic(__FILE__, __LINE__, "Missing primary heap arena");
    newHi = reinterpret_cast< void* >(arenaLo + 0xE00000);
    OSSetMEM2ArenaHi(newHi);
    JKRHeap::setAltAramStartAdr(arenaLo);
    pHeap = JKRExpHeap::create(newHi, arenaHi - reinterpret_cast< uintptr_t >(newHi), JKRHeap::sRootHeap, true);

    if (MR::isEqualCurrentHeap(pHeap)) {
        JKRHeap::sRootHeap->becomeCurrentHeap();
    }

    HeapMemoryWatcher::sRootHeapGDDR3 = pHeap;
}

