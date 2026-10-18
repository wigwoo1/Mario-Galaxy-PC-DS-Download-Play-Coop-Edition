struct OSThread;

namespace {
thread_local OSThread* assignedThread = nullptr;
thread_local unsigned char hostThreadIdentity;
}

extern "C" OSThread* OSGetCurrentThread(void) {
    return assignedThread ? assignedThread : reinterpret_cast<OSThread*>(&hostThreadIdentity);
}

void nativeAssignCurrentThread(OSThread* thread) {
    assignedThread = thread;
}
