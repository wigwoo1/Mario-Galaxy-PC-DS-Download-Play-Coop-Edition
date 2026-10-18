#include <revolution/os/OSAlarm.h>
#include <revolution/os/OSTime.h>
#ifdef nullptr
#undef nullptr
#endif
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace {
using Clock = std::chrono::steady_clock;
constexpr uint64_t kTicksPerSecond = 60750000;

Clock::duration ticksToDuration(uint64_t ticks) {
    // Bound host waits to ten years. This leaves ample headroom for game timers
    // and avoids overflowing the host clock representation on malformed input.
    constexpr uint64_t maxTicks = kTicksPerSecond * 60 * 60 * 24 * 365 * 10;
    ticks = std::min(ticks, maxTicks);
    const uint64_t seconds = ticks / kTicksPerSecond;
    const uint64_t remainder = ticks % kTicksPerSecond;
    const uint64_t nanoseconds = seconds * 1000000000ULL + remainder * 1000000000ULL / kTicksPerSecond;
    return std::chrono::duration_cast<Clock::duration>(std::chrono::nanoseconds(nanoseconds));
}

Clock::time_point deadlineAfter(OSTime ticks) {
    return Clock::now() + ticksToDuration(ticks > 0 ? uint64_t(ticks) : 0);
}

struct AlarmState {
    uint64_t generation = 0;
    bool armed = false;
    bool active = false;
    std::thread::id callbackThread{};
    OSAlarmHandler handler = nullptr;
};

struct AlarmEntry {
    Clock::time_point deadline;
    OSAlarm* alarm;
    std::shared_ptr<AlarmState> state;
    uint64_t generation;
    bool operator>(const AlarmEntry& other) const { return deadline > other.deadline; }
};

class AlarmScheduler {
public:
    AlarmScheduler() : worker_([this] { run(); }) {}
    ~AlarmScheduler() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopping_ = true;
            changed_.notify_all();
        }
        if (worker_.joinable()) worker_.join();
    }

    void create(OSAlarm* alarm) {
        if (!alarm) return;
        std::unique_lock<std::mutex> lock(mutex_);
        auto found = states_.find(alarm);
        if (found != states_.end()) {
            auto state = found->second;
            state->armed = false;
            ++state->generation;
            alarm->handler = nullptr;
            changed_.notify_all();
            if (state->active && state->callbackThread != std::this_thread::get_id())
                changed_.wait(lock, [&] { return !state->active; });
            states_.erase(alarm);
        }
        alarm->handler = nullptr;
        alarm->tag = 0;
        alarm->fire = 0;
        alarm->prev = nullptr;
        alarm->next = nullptr;
        alarm->period = 0;
        alarm->start = 0;
        alarm->userData = nullptr;
    }

    void set(OSAlarm* alarm, OSTime delay, OSAlarmHandler handler) {
        if (!alarm) return;
        std::lock_guard<std::mutex> lock(mutex_);
        auto state = getState(alarm);
        ++state->generation;
        state->handler = handler;
        state->armed = handler != nullptr;
        alarm->period = 0;
        alarm->fire = addSaturated(OSGetTime(), delay > 0 ? delay : 0);
        alarm->handler = handler;
        alarm->prev = nullptr;
        alarm->next = nullptr;
        if (state->armed) queue_.push({deadlineAfter(delay), alarm, state, state->generation});
        changed_.notify_all();
    }

    void setPeriodic(OSAlarm* alarm, OSTime start, OSTime period, OSAlarmHandler handler) {
        if (!alarm) return;
        std::lock_guard<std::mutex> lock(mutex_);
        auto state = getState(alarm);
        ++state->generation;
        state->handler = handler;
        state->armed = handler != nullptr && period > 0;
        alarm->period = period;
        alarm->start = start;
        alarm->handler = state->armed ? handler : nullptr;
        alarm->prev = nullptr;
        alarm->next = nullptr;

        const OSTime nowTicks = OSGetTime();
        // OSGetTick is a low 32-bit view of this clock. If a caller supplied
        // that value after wrap, unwrap it to the nearest 32-bit era.
        if (start >= 0 && uint64_t(start) <= UINT32_MAX && nowTicks > UINT32_MAX) {
            uint64_t candidate = (uint64_t(nowTicks) & ~uint64_t(UINT32_MAX)) | uint32_t(start);
            constexpr uint64_t wrap = uint64_t(UINT32_MAX) + 1;
            if (candidate + wrap / 2 < uint64_t(nowTicks)) candidate += wrap;
            else if (candidate > uint64_t(nowTicks) + wrap / 2 && candidate >= wrap) candidate -= wrap;
            start = candidate > uint64_t(INT64_MAX) ? INT64_MAX : OSTime(candidate);
        }

        OSTime first = start < 0 ? nowTicks : start;
        if (period > 0 && first <= nowTicks) {
            const uint64_t elapsed = uint64_t(nowTicks) - uint64_t(first);
            const uint64_t steps = elapsed / uint64_t(period) + 1;
            if (steps > uint64_t(INT64_MAX) / uint64_t(period)) first = INT64_MAX;
            else {
                const uint64_t advance = steps * uint64_t(period);
                first = advance > uint64_t(INT64_MAX) - uint64_t(first) ? INT64_MAX : OSTime(uint64_t(first) + advance);
            }
        }
        alarm->fire = first;
        if (state->armed) queue_.push({deadlineAfter(first - nowTicks), alarm, state, state->generation});
        changed_.notify_all();
    }

    void cancel(OSAlarm* alarm) {
        if (!alarm) return;
        std::unique_lock<std::mutex> lock(mutex_);
        auto found = states_.find(alarm);
        alarm->handler = nullptr;
        if (found == states_.end()) return;
        auto state = found->second;
        state->armed = false;
        ++state->generation;
        changed_.notify_all();
        if (state->active && state->callbackThread == std::this_thread::get_id()) return;
        changed_.wait(lock, [&] { return !state->active; });
        auto current = states_.find(alarm);
        if (current != states_.end() && current->second == state) states_.erase(current);
    }

    void setTag(OSAlarm* alarm, u32 tag) {
        if (!alarm) return;
        std::lock_guard<std::mutex> lock(mutex_);
        alarm->tag = tag;
    }

private:
    static OSTime addSaturated(OSTime value, OSTime delta) {
        if (delta <= 0) return value;
        return value > INT64_MAX - delta ? INT64_MAX : value + delta;
    }

    std::shared_ptr<AlarmState> getState(OSAlarm* alarm) {
        auto& value = states_[alarm];
        if (!value) value = std::make_shared<AlarmState>();
        return value;
    }

    void run() {
        std::unique_lock<std::mutex> lock(mutex_);
        while (!stopping_) {
            while (!queue_.empty()) {
                const auto& top = queue_.top();
                if (top.state->armed && top.state->generation == top.generation) break;
                queue_.pop();
            }
            if (queue_.empty()) {
                changed_.wait(lock, [&] { return stopping_ || !queue_.empty(); });
                continue;
            }
            const AlarmEntry entry = queue_.top();
            if (changed_.wait_until(lock, entry.deadline) != std::cv_status::timeout) continue;
            if (stopping_) break;
            if (queue_.empty() || queue_.top().generation != entry.generation || queue_.top().alarm != entry.alarm) continue;
            queue_.pop();
            auto state = entry.state;
            if (!state->armed || state->generation != entry.generation) continue;

            OSAlarmHandler handler = state->handler;
            if (!handler) {
                state->armed = false;
                continue;
            }
            state->active = true;
            state->callbackThread = std::this_thread::get_id();
            const OSTime period = entry.alarm->period;
            if (period <= 0) {
                state->armed = false;
                entry.alarm->handler = nullptr;
            } else {
                const OSTime nowTicks = OSGetTime();
                OSTime next = addSaturated(entry.alarm->fire, period);
                if (next <= nowTicks && next != INT64_MAX) {
                    const uint64_t missed = uint64_t(nowTicks) - uint64_t(next);
                    const uint64_t steps = missed / uint64_t(period) + 1;
                    if (steps > uint64_t(INT64_MAX) / uint64_t(period)) next = INT64_MAX;
                    else {
                        const uint64_t advance = steps * uint64_t(period);
                        next = advance > uint64_t(INT64_MAX) - uint64_t(next) ? INT64_MAX : OSTime(uint64_t(next) + advance);
                    }
                }
                entry.alarm->fire = next;
                queue_.push({deadlineAfter(next - nowTicks), entry.alarm, state, entry.generation});
            }

            lock.unlock();
            try { handler(entry.alarm, nullptr); } catch (...) { /* C callbacks may not unwind through the scheduler thread. */ }
            lock.lock();
            state->active = false;
            state->callbackThread = {};
            if (!state->armed) {
                auto current = states_.find(entry.alarm);
                if (current != states_.end() && current->second == state) states_.erase(current);
            }
            changed_.notify_all();
        }
    }

    std::mutex mutex_;
    std::condition_variable changed_;
    std::priority_queue<AlarmEntry, std::vector<AlarmEntry>, std::greater<AlarmEntry>> queue_;
    std::map<OSAlarm*, std::shared_ptr<AlarmState>> states_;
    bool stopping_ = false;
    std::thread worker_;
};

AlarmScheduler& scheduler() {
    static AlarmScheduler value;
    return value;
}
}

extern "C" void OSCreateAlarm(OSAlarm* alarm) { scheduler().create(alarm); }
extern "C" void OSSetAlarm(OSAlarm* alarm, OSTime ticks, OSAlarmHandler handler) { scheduler().set(alarm, ticks, handler); }
extern "C" void OSSetPeriodicAlarm(OSAlarm* alarm, OSTime start, OSTime period, OSAlarmHandler handler) { scheduler().setPeriodic(alarm, start, period, handler); }
extern "C" void OSCancelAlarm(OSAlarm* alarm) { scheduler().cancel(alarm); }
extern "C" void OSSetAlarmTag(OSAlarm* alarm, u32 tag) { scheduler().setTag(alarm, tag); }
extern "C" void OSSetAlarmUserData(OSAlarm* alarm, void* data) { if (alarm) alarm->userData = data; }
extern "C" void* OSGetAlarmUserData(const OSAlarm* alarm) { return alarm ? alarm->userData : nullptr; }
