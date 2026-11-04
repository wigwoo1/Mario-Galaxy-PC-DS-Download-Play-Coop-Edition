#include <Native/HeapPlacement.hpp>
#include "JSystem/JAudio2/JASDvdThread.hpp"
#include "JSystem/JAudio2/JASTaskThread.hpp"
#include "JSystem/JKernel/JKRSolidHeap.hpp"

namespace JASDvd {
    static JASTaskThread* sThread;
    // Base destruction runs in reverse order: retain the published reference
    // until JASTaskThread/JKRThread have drained/cancelled and joined the worker.
    // Creation/destruction are serialized; the factory permits one live worker.
    class NativeDvdThreadReference {
    protected:
        ~NativeDvdThreadReference() { sThread = nullptr; }
    };
    class NativeDvdTaskThread final : private NativeDvdThreadReference, public JASTaskThread {
    public:
        using JASTaskThread::JASTaskThread;
    };
    static_assert(sizeof(NativeDvdTaskThread) == sizeof(JASTaskThread), "DVD worker native ABI changed");
};  // namespace JASDvd

JASTaskThread* JASDvd::getThreadPointer() {
    return JASDvd::sThread;
}

bool JASDvd::createThread(s32 priority, int msgCount, u32 stackSize) {
    if (sThread != NULL) {
        return false;
    }

    sThread = new (JASDram, 0) NativeDvdTaskThread(priority, msgCount, stackSize);

    JKRHeap* pCurrentHeap = JKRGetSystemHeap();

    sThread->setCurrentHeap(pCurrentHeap);
    sThread->resume();
    return true;
}
