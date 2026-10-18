#include <Native/WiiSoundMode.hpp>
#include <revolution/sc.h>
#include <cassert>
#include <cstdio>

int main() {
    assert(SCGetSoundMode() == SC_SOUND_MODE_DEFAULT);
    assert(OSGetSoundMode() == SC_SOUND_MODE_DEFAULT);
    assert(Native::setWiiSoundMode(Native::WiiSoundMode::Mono));
    assert(SCGetSoundMode() == SC_SOUND_MODE_MONO);
    assert(OSGetSoundMode() == SC_SOUND_MODE_MONO);
    assert(Native::setWiiSoundMode(Native::WiiSoundMode::Stereo));
    assert(SCGetSoundMode() == SC_SOUND_MODE_STEREO);
    assert(OSGetSoundMode() == SC_SOUND_MODE_STEREO);
    assert(Native::setWiiSoundMode(Native::WiiSoundMode::EncodedSurround));
    assert(SCGetSoundMode() == SC_SOUND_MODE_SURROUND);
    assert(OSGetSoundMode() == SC_SOUND_MODE_SURROUND);
    assert(!Native::setWiiSoundMode(static_cast<Native::WiiSoundMode>(3)));
    assert(SCGetSoundMode() == SC_SOUND_MODE_SURROUND);
    assert(OSGetSoundMode() == SC_SOUND_MODE_SURROUND);
    std::puts("PASS Wii sound-mode default, selection, and invalid-mode rejection");
}
