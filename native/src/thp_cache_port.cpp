#include <revolution/os/OSCache.h>

#include <atomic>

// The Wii THP path toggles a dedicated 16 MiB local cache and its PPC-specific
// HID2/DBAT mapping. The native wrapper stores decoder inputs in ordinary host
// allocations instead, so there is no local-cache mapping to enable or flush.
// Keep the call boundaries ordered for surrounding host-side buffer work.
extern "C" void LCEnable(void) {
    std::atomic_thread_fence(std::memory_order_seq_cst);
}

extern "C" void LCDisable(void) {
    std::atomic_thread_fence(std::memory_order_seq_cst);
}
