#include <Game/Util/MemoryUtil.hpp>
#include <Game/Util/MutexHolder.hpp>
#include <JSystem/JAudio2/JASMutex.hpp>
#include <JSystem/JKernel/JKRHeap.hpp>
namespace MR {
    CurrentHeapRestorer::CurrentHeapRestorer(JKRHeap* pHeap) {
        OSLockMutex(&MR::MutexHolder< 1 >::sMutex);
        _0 = JKRHeap::sCurrentHeap;
        MR::becomeCurrentHeap(pHeap);
    }

    CurrentHeapRestorer::~CurrentHeapRestorer() {
        MR::becomeCurrentHeap(_0);
        OSUnlockMutex(&MR::MutexHolder< 1 >::sMutex);
    }

    void becomeCurrentHeap(JKRHeap* pHeap) {
        JASMutexLock lock(&MR::MutexHolder< 1 >::sMutex);
        pHeap->becomeCurrentHeap();
    }


}

#include <JSystem/JKernel/JKRExpHeap.hpp>
namespace MR {
    bool isEqualCurrentHeap(JKRHeap* pHeap) {
        return JKRHeap::sCurrentHeap == pHeap;
    }

    void adjustHeapSize(JKRExpHeap* pHeap, const char* pParam2) {
        pHeap->adjustSize();
    }


}
