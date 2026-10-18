#include <Game/System/DrawSyncManager.hpp>
#include <memory>
namespace { enum class NativeDrawMessageKind { BreakPoint, Token, Shutdown }; struct NativeDrawMessage { NativeDrawMessageKind kind; void* pointer; }; }
void* Fifo::pop() {
    void* pValue = mArray[mLoopIdx];
    mLoopIdx = getLoopIdx(mLoopIdx + 1);
    return pValue;
}

u32 Fifo::getLoopIdx(u32 index) {
    if (index >= mCount + 1) {
        index = 0;
    }

    return index;
}

u32 Fifo::getCount() {
    if (mLoopIdx <= mWriteIndex) {
        return mWriteIndex - mLoopIdx;
    }

    return mWriteIndex + mCount + 1 - mLoopIdx;
}

void* DrawSyncManager::threadFunc(void* pArg) {
    DrawSyncManager* pManager = static_cast< DrawSyncManager* >(pArg);

    while (true) {
        OSMessage message;
        OSReceiveMessage(&pManager->mQueue, &message, OS_MESSAGE_BLOCK);

        std::unique_ptr<NativeDrawMessage> payload(static_cast<NativeDrawMessage*>(message));
        if (payload->kind == NativeDrawMessageKind::BreakPoint) {
            pManager->mFifo->push(payload->pointer);
            const bool aborted = pManager->mAborted;

            if (pManager->mFifo->getCount() == 2) {
                GXEnableBreakPt(payload->pointer);
            }
        } else if (payload->kind == NativeDrawMessageKind::Token) {
            pManager->mFifo->pop();
            const bool aborted = pManager->mAborted;

            u32 count = pManager->mFifo->getCount();
            if (count != 0) {
                if (count == 1) {
                    GXDisableBreakPt();
                } else if (count >= 2) {
                    GXEnableBreakPt(pManager->mFifo->peek(1));
                }
            }
        } else {
            break;
        }
    }

    return nullptr;
}


