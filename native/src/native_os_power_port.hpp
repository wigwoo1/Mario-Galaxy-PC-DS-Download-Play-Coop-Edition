#pragma once
#include <revolution/types.h>

enum class NativeSystemAction : u32 {
    None = 0,
    Restart,
    ReturnToMenu,
    RebootSystem,
    ShutdownSystem
};

NativeSystemAction nativeConsumeSystemAction(u32* resetCode = nullptr);
void nativeNotifyPowerOff();
