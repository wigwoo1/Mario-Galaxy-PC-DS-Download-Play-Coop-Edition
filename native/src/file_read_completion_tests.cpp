#include <Game/System/FileHolder.hpp>
#include <cassert>
#include <atomic>
#include <future>
#include <cstdio>
// Allocation and DVD access are outside this completion-boundary test.
FileHolderFileEntry::FileHolderFileEntry(const char*, JKRHeap* heap, void*)
    : mEntryNum(0), mContext(nullptr), mHeap(heap), mState(0), mQueue{}, mMessage{}, mContextSet(true) {}
FileHolderFileEntry::~FileHolderFileEntry() {}
int main() {
    int payload = 42;
    FileHolderFileEntry early("early", nullptr, nullptr);
    early.setContext(&payload, nullptr);
    early.waitReadDone(); early.waitReadDone();
    assert(early.mContext == &payload && early.mState == 2);
    FileHolderFileEntry shared("shared", nullptr, nullptr);
    std::atomic<unsigned> started{0};
    auto reader = [&] { ++started; shared.waitReadDone(); assert(shared.mContext == &payload && shared.mState == 2); };
    auto first = std::async(std::launch::async, reader);
    auto second = std::async(std::launch::async, reader);
    while (started.load() != 2) std::this_thread::yield();
    shared.setContext(&payload, nullptr);
    for(auto* result : {&first, &second}) { assert(result->wait_for(std::chrono::seconds(2)) == std::future_status::ready); result->get(); }
    // Completion must remain visible after both waiting readers return.
    shared.waitReadDone(); assert(shared.mState == 2);
    std::puts("PASS actual file completion methods: publish-before-wait, multiple readers, repeated waits, and payload visibility");
}
