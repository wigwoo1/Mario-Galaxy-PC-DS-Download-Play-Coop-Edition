#include <Game/Speaker/SpkSpeakerCtrl.hpp>

// A PC host has no Wii Remote speaker. Keep the original SpkSystem lifecycle
// safe while making that optional output path explicitly unavailable.
namespace SpkSpeakerCtrl {
void setMixingBuffer(SpkMixingBuffer*) {}
void setup() {}
void connect(s32) {}
void disconnect(s32) {}
void setSpeakerOn(s32) {}
void setSpeakerOnCallback(s32, s32) {}
void setSpeakerPlay(s32) {}
void startPlayCallback(s32, s32) {}
void setSpeakerOff(s32) {}
void retryConnection(s32) {}
void reconnect(s32) {}
void framework() {}
void reconnectProcess(s32) {}
void initReconnect(s32) {}
void continuousUsingProcess(s32) {}
void updateSpeaker(OSAlarm*, OSContext*) {}
bool isEnable(s32) { return false; }
void extensionProcess(s32, s32) {}
f32 getDeviceVolume(s32) { return 0.0f; }
} // namespace SpkSpeakerCtrl
