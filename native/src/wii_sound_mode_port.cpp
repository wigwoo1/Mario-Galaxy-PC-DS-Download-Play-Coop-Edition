#include <Native/WiiSoundMode.hpp>
#include <revolution/sc.h>
#include <atomic>

namespace {
std::atomic<std::uint8_t> currentMode{SC_SOUND_MODE_DEFAULT};
}

namespace Native {

WiiSoundMode wiiSoundMode() noexcept {
    return static_cast<WiiSoundMode>(currentMode.load(std::memory_order_relaxed));
}

bool setWiiSoundMode(WiiSoundMode mode) noexcept {
    const auto value = static_cast<std::uint8_t>(mode);
    if (value > SC_SOUND_MODE_SURROUND)
        return false;
    currentMode.store(value, std::memory_order_relaxed);
    return true;
}

} // namespace Native

extern "C" u8 SCGetSoundMode(void) {
    return static_cast<u8>(Native::wiiSoundMode());
}

extern "C" u32 OSGetSoundMode(void) {
    return static_cast<u32>(Native::wiiSoundMode());
}
