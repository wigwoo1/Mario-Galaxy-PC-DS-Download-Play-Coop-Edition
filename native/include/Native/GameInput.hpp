#pragma once

// Bind the actual host game HWND before entering nativeOriginalGameMain.
void nativeBindGameInputWindow(void* window);
// Pump keyboard/mouse state once at the native GameSystem frame boundary.
void nativePumpGameInputFrame();
// Configure a virtual-key code and XInput button mask for a held shake impulse.
void nativeSetGameInputShakeBindings(int virtualKey, unsigned short buttonMask);
// Set the rendered viewport in host client coordinates; non-positive extents reject.
// Reset to full client-area mapping by passing (0, 0, clientWidth, clientHeight).
bool nativeSetGamePointerViewport(int x, int y, int width, int height);
// Reset the host VI idle timer after keyboard, controller, or pointer activity.
extern "C" void nativeNotifyVideoActivity();
