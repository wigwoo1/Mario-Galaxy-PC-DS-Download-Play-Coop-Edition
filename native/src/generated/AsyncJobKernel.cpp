#include <Game/Util/Functor.hpp>
#include <Game/System/FunctionAsyncExecutor.hpp>
#include <JSystem/JKernel/JKRHeap.hpp>
FunctionAsyncExecInfo::FunctionAsyncExecInfo(MR::FunctorBase* pFunc, int priority, const char* pName)
    : mFunc(pFunc), mPriority(priority), mName(pName), mIsEnd(false) {
    OSInitMessageQueue(&mQueue, &mMessage, 1);
}

FunctionAsyncExecInfo::~FunctionAsyncExecInfo() {
    if (mFunc != nullptr && !mFunc->dispose()) {
        JKRHeap::free(mFunc, nullptr);
    }
}

void FunctionAsyncExecInfo::execute() {
    OSThread* pThread = OSGetCurrentThread();
    OSPriority priorityPrev = OSGetThreadPriority(pThread);

    OSSetThreadPriority(pThread, mPriority);
    (*mFunc)();
    OSSetThreadPriority(pThread, priorityPrev);
}

FunctionAsyncExecutorOnMainThread::FunctionAsyncExecutorOnMainThread(OSThread* pThread) : mThread(pThread), _0(false) {
    OSInitMessageQueue(&mQueue, mMsgArray, ARRAY_SIZE(mMsgArray));
}

void FunctionAsyncExecutorOnMainThread::update() {
    OSMessage msg;
    FunctionAsyncExecInfo* info;

    if (OSReceiveMessage(&mQueue, &msg, OS_MESSAGE_NOBLOCK)) {
        _0 = true;

        info = static_cast< FunctionAsyncExecInfo* >(msg);
        info->mPriority = OSGetThreadPriority(OSGetCurrentThread());
        info->execute();

        info->mIsEnd = true;
        OSSendMessage(&info->mQueue, nullptr, OS_MESSAGE_NOBLOCK);
    }
}

