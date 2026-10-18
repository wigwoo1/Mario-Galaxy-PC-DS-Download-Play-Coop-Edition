#pragma once
#include <revolution/wpad.h>

// Called by the host window/input pump when its game window attaches or closes.
void nativeSetHostWpadConnected(bool connected);
// Deliver queued controller, extension and information callbacks from the frame pump.
void nativeDispatchWpadCallbacks();
