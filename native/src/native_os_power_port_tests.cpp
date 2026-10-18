#include "native_os_power_port.hpp"
#include <revolution/os/OSReset.h>
#include <revolution/os/OSResetSW.h>
#include <cassert>
#include <cstdio>

namespace {
unsigned calls = 0;
void powerCallback() { ++calls; }
}

int main() {
    assert(nativeConsumeSystemAction() == NativeSystemAction::None);
    assert(OSSetPowerCallback(powerCallback) == nullptr);
    nativeNotifyPowerOff();
    assert(calls == 1);
    assert(OSSetPowerCallback(nullptr) == powerCallback);
    nativeNotifyPowerOff();
    assert(calls == 1);
    assert(OSGetResetButtonState() == 0);

    u32 code = 0;
    OSRestart(0x1234);
    assert(nativeConsumeSystemAction(&code) == NativeSystemAction::Restart && code == 0x1234);
    assert(nativeConsumeSystemAction() == NativeSystemAction::None);
    OSReturnToMenu();
    assert(nativeConsumeSystemAction() == NativeSystemAction::ReturnToMenu);
    OSRebootSystem();
    assert(nativeConsumeSystemAction() == NativeSystemAction::RebootSystem);
    OSShutdownSystem();
    assert(nativeConsumeSystemAction() == NativeSystemAction::ShutdownSystem);
    std::puts("PASS host Wii OS power adapter: callback replacement/notification, absent reset switch and deferred restart/menu/reboot/shutdown requests");
}
