#pragma once
#include <condition_variable>
#include <mutex>

namespace Native {
// A file remains completed after any number of readers observe it.
class ReadCompletion {
    std::mutex mutex;
    std::condition_variable ready;
    bool completed = false;
public:
    void wait() {
        std::unique_lock<std::mutex> lock(mutex);
        ready.wait(lock, [this] { return completed; });
    }
    template<class Publish> void publish(Publish publishFields) {
        { std::lock_guard<std::mutex> lock(mutex); publishFields(); completed = true; }
        ready.notify_all();
    }
};
}
