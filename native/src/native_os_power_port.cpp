#include "native_os_power_port.hpp"
#include <revolution/os/OSReset.h>
#include <revolution/os/OSResetSW.h>
#include <atomic>
#include <mutex>

namespace {
void defaultPowerCallback() {}
std::mutex powerMutex;
OSPowerCallback powerCallback = defaultPowerCallback;
std::atomic<u32> pendingAction{static_cast<u32>(NativeSystemAction::None)};
std::atomic<u32> pendingResetCode{0};
}

extern "C" OSPowerCallback OSSetPowerCallback(OSPowerCallback callback) {
    std::lock_guard<std::mutex> lock(powerMutex);
    OSPowerCallback previous = powerCallback == defaultPowerCallback ? nullptr : powerCallback;
    powerCallback = callback ? callback : defaultPowerCallback;
    return previous;
}

extern "C" BOOL OSGetResetButtonState(void) {
    // Desktop keyboards have no Wii reset switch; reset and pause inputs remain distinct.
    return 0;
}

extern "C" void OSRestart(u32 resetCode) {
    pendingResetCode.store(resetCode, std::memory_order_relaxed);
    pendingAction.store(static_cast<u32>(NativeSystemAction::Restart), std::memory_order_release);
}

extern "C" void OSReturnToMenu(void) {
    pendingResetCode.store(0, std::memory_order_relaxed);
    pendingAction.store(static_cast<u32>(NativeSystemAction::ReturnToMenu), std::memory_order_release);
}

extern "C" void OSRebootSystem(void) {
    pendingResetCode.store(0, std::memory_order_relaxed);
    pendingAction.store(static_cast<u32>(NativeSystemAction::RebootSystem), std::memory_order_release);
}

extern "C" void OSShutdownSystem(void) {
    pendingResetCode.store(0, std::memory_order_relaxed);
    pendingAction.store(static_cast<u32>(NativeSystemAction::ShutdownSystem), std::memory_order_release);
}

void nativeNotifyPowerOff() {
    OSPowerCallback callback;
    {
        std::lock_guard<std::mutex> lock(powerMutex);
        callback = powerCallback;
    }
    if (callback) callback();
}

NativeSystemAction nativeConsumeSystemAction(u32* resetCode) {
    const u32 action = pendingAction.exchange(static_cast<u32>(NativeSystemAction::None), std::memory_order_acq_rel);
    if (resetCode) *resetCode = pendingResetCode.load(std::memory_order_relaxed);
    return static_cast<NativeSystemAction>(action);
}
