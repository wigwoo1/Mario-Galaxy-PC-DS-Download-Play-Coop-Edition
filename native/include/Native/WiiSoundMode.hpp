#pragma once

#include <cstdint>

namespace Native {

// Original Wii/JAudio mix mode, distinct from the host endpoint's channel count.
enum class WiiSoundMode : std::uint8_t {
    Mono = 0,
    Stereo = 1,
    EncodedSurround = 2,
};

WiiSoundMode wiiSoundMode() noexcept;
bool setWiiSoundMode(WiiSoundMode mode) noexcept;

} // namespace Native
