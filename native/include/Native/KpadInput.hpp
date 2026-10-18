#pragma once
#include <revolution/kpad.h>
struct NativeKpadCalibration {
 f32 posRadius{},posSensitivity{},horizonRadius{},horizonSensitivity{};
 f32 distanceRadius{},distanceSensitivity{},accelerationRadius{},accelerationSensitivity{},sensorHeight{};
};
// Platform producers supply fully processed KPAD samples, including edge bits.
// Four independent channels; a full queue rejects input without discarding edges.
bool nativeSubmitKpadSample(s32 channel,const KPADStatus& sample);
// Device-state path derives edge/repeat bits using monotonic seconds; do not mix submission paths on a channel.
bool nativeSubmitKpadState(s32 channel,const KPADStatus& state,double seconds);
bool nativeReadKpadCalibration(s32 channel,NativeKpadCalibration* output);
