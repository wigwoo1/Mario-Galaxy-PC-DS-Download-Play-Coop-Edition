#include <revolution/os/OSAlarm.h>
#include <revolution/os/OSTime.h>
#ifdef nullptr
#undef nullptr
#endif
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <thread>

namespace {
constexpr OSTime ms(unsigned value) { return OSTime(value) * 60750; }
std::mutex mutex;
std::condition_variable changed;
unsigned calls = 0;
bool entered = false;
bool releaseCallback = false;
OSAlarm* expectedAlarm = nullptr;

void oneShot(OSAlarm* alarm, OSContext* context) {
    std::lock_guard<std::mutex> lock(mutex);
    assert(alarm == expectedAlarm && context == nullptr);
    ++calls;
    changed.notify_all();
}
void periodic(OSAlarm* alarm, OSContext* context) {
    std::lock_guard<std::mutex> lock(mutex);
    assert(alarm == expectedAlarm && context == nullptr);
    ++calls;
    changed.notify_all();
}
void blocking(OSAlarm*, OSContext*) {
    std::unique_lock<std::mutex> lock(mutex);
    entered = true;
    changed.notify_all();
    changed.wait(lock, [] { return releaseCallback; });
}
bool waitCalls(unsigned target, std::chrono::milliseconds timeout = std::chrono::milliseconds(1000)) {
    std::unique_lock<std::mutex> lock(mutex);
    return changed.wait_for(lock, timeout, [&] { return calls >= target; });
}
}

int main() {
    OSAlarm alarm{};
    expectedAlarm = &alarm;
    OSCreateAlarm(&alarm);
    assert(alarm.handler == nullptr && alarm.tag == 0 && alarm.period == 0 && !alarm.next && !alarm.prev);
    int payload = 7;
    OSSetAlarmUserData(&alarm, &payload);
    assert(OSGetAlarmUserData(&alarm) == &payload);
    OSSetAlarmTag(&alarm, 42);
    assert(alarm.tag == 42);

    calls = 0;
    OSSetAlarm(&alarm, ms(8), oneShot);
    assert(waitCalls(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    assert(calls == 1 && alarm.handler == nullptr);

    calls = 0;
    OSSetAlarm(&alarm, ms(40), oneShot);
    OSCancelAlarm(&alarm);
    std::this_thread::sleep_for(std::chrono::milliseconds(55));
    assert(calls == 0 && alarm.handler == nullptr);

    calls = 0;
    const OSTime start = OSGetTime() + ms(2);
    OSSetPeriodicAlarm(&alarm, start, ms(5), periodic);
    assert(waitCalls(3));
    OSCancelAlarm(&alarm);
    const unsigned stoppedAt = calls;
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    assert(calls == stoppedAt && stoppedAt >= 3 && alarm.handler == nullptr);

    OSSetAlarm(&alarm, ms(5), blocking);
    {
        std::unique_lock<std::mutex> lock(mutex);
        assert(changed.wait_for(lock, std::chrono::seconds(1), [] { return entered; }));
    }
    std::atomic<bool> cancelReturned{false};
    std::thread canceller([&] { OSCancelAlarm(&alarm); cancelReturned.store(true, std::memory_order_release); });
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    assert(!cancelReturned.load(std::memory_order_acquire));
    {
        std::lock_guard<std::mutex> lock(mutex);
        releaseCallback = true;
        changed.notify_all();
    }
    canceller.join();
    assert(cancelReturned.load(std::memory_order_acquire));

    std::puts("PASS native OS alarms: one-shot, periodic phase, user data/tag, cancellation, and in-flight callback synchronization");
}
