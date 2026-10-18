#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace Native {
using GxCommandObserver = void (*)(const std::uint8_t*, std::size_t, bool);
inline std::atomic<GxCommandObserver>& gxCommandObserverSlot() noexcept {
    static std::atomic<GxCommandObserver> observer{nullptr};
    return observer;
}
inline GxCommandObserver exchangeGxCommandObserver(GxCommandObserver next) noexcept {
    return gxCommandObserverSlot().exchange(next, std::memory_order_acq_rel);
}
inline void notifyGxCommandObserver(const std::uint8_t* start, std::size_t size,
                                    bool reset = false) {
    if (auto observer = gxCommandObserverSlot().load(std::memory_order_acquire))
        observer(start, size, reset);
}
}
