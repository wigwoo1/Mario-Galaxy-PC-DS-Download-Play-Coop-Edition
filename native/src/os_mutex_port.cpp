#include <revolution/os/OSMutex.h>
#ifdef nullptr
#undef nullptr
#endif
#include <map>
#include <memory>
#include <mutex>
#include <thread>

extern "C" OSThread* OSGetCurrentThread(void);

namespace {
struct MutexState {
    std::recursive_mutex gate;
    std::mutex metadata;
    std::thread::id owner;
    u32 recursion = 0;
};

struct MutexRegistry {
    std::mutex mutex;
    std::map<OSMutex*, std::shared_ptr<MutexState>> states;
};

MutexRegistry& mutexRegistry() {
    // Original game globals can initialize Wii mutexes from other translation
    // units before namespace-scope C++ objects in this file are constructed.
    // Defer the registry itself until first use to avoid that startup-order race.
    static MutexRegistry registry;
    return registry;
}

std::shared_ptr<MutexState> stateFor(OSMutex* mutex) {
    if (!mutex) return {};
    auto& registry = mutexRegistry();
    std::lock_guard<std::mutex> lock(registry.mutex);
    auto& state = registry.states[mutex];
    if (!state) state = std::make_shared<MutexState>();
    return state;
}

OSThread* currentOwner() {
    return OSGetCurrentThread();
}

void acquired(OSMutex* mutex, const std::shared_ptr<MutexState>& state) {
    std::lock_guard<std::mutex> lock(state->metadata);
    if (state->recursion == 0) state->owner = std::this_thread::get_id();
    ++state->recursion;
#if defined(GALAXY_OSMUTEX_HAS_LAYOUT)
    mutex->thread = currentOwner();
    mutex->count = static_cast<s32>(state->recursion);
#else
    (void)mutex;
#endif
}
}

extern "C" void OSInitMutex(OSMutex* mutex) {
    if (!mutex) return;
    (void)stateFor(mutex);
#if defined(GALAXY_OSMUTEX_HAS_LAYOUT)
    mutex->thread = nullptr;
    mutex->count = 0;
#endif
}

extern "C" void OSLockMutex(OSMutex* mutex) {
    auto state = stateFor(mutex);
    if (!state) return;
    state->gate.lock();
    acquired(mutex, state);
}

extern "C" BOOL OSTryLockMutex(OSMutex* mutex) {
    auto state = stateFor(mutex);
    if (!state || !state->gate.try_lock()) return 0;
    acquired(mutex, state);
    return 1;
}

extern "C" void OSUnlockMutex(OSMutex* mutex) {
    auto state = stateFor(mutex);
    if (!state) return;
    {
        std::lock_guard<std::mutex> lock(state->metadata);
        if (state->recursion == 0 || state->owner != std::this_thread::get_id()) return;
        --state->recursion;
#if defined(GALAXY_OSMUTEX_HAS_LAYOUT)
        mutex->count = static_cast<s32>(state->recursion);
        if (state->recursion == 0) mutex->thread = nullptr;
#endif
        if (state->recursion == 0) state->owner = {};
    }
    state->gate.unlock();
}
