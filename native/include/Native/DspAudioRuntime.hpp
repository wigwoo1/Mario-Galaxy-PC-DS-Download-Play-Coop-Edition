// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
// Owner: the native JAudio thread. Configure before JASDsp::boot; boot's
// verified handshake creates the renderer. DSPInit/stop unregister before
// releasing owned state. Service pending completion before waiting for mail.
void nativeConfigureDspAudioRuntime();
void nativeStopDspAudioRuntime();
bool nativeDspAudioRuntimeActive();
bool nativeDspAudioRuntimeFailed();
