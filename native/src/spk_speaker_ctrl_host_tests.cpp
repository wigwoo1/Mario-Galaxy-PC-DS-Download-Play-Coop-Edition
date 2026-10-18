#include <Game/Speaker/SpkSpeakerCtrl.hpp>
#include <cassert>
#include <cstdio>

int main() {
    SpkSpeakerCtrl::setup();
    SpkSpeakerCtrl::setMixingBuffer(nullptr);
    for (s32 channel = -1; channel < WPAD_MAX_CONTROLLERS + 1; ++channel) {
        SpkSpeakerCtrl::connect(channel);
        SpkSpeakerCtrl::disconnect(channel);
        SpkSpeakerCtrl::reconnect(channel);
        SpkSpeakerCtrl::setSpeakerOn(channel);
        SpkSpeakerCtrl::setSpeakerPlay(channel);
        SpkSpeakerCtrl::setSpeakerOff(channel);
        SpkSpeakerCtrl::retryConnection(channel);
        SpkSpeakerCtrl::reconnectProcess(channel);
        SpkSpeakerCtrl::initReconnect(channel);
        SpkSpeakerCtrl::continuousUsingProcess(channel);
        assert(!SpkSpeakerCtrl::isEnable(channel));
        assert(SpkSpeakerCtrl::getDeviceVolume(channel) == 0.0f);
    }
    SpkSpeakerCtrl::extensionProcess(0, 0);
    SpkSpeakerCtrl::framework();
    std::puts("PASS PC Wii-speaker adapter: lifecycle remains safe and unsupported remote speakers stay silent");
}
