#include <Native/ThpAudio.hpp>

#include <array>
#include <cassert>
#include <cstdio>
#include <vector>

namespace {

void put16(std::vector<u8>& bytes, std::size_t at, u16 value) {
    bytes[at] = static_cast<u8>(value >> 8);
    bytes[at + 1] = static_cast<u8>(value);
}

void put32(std::vector<u8>& bytes, std::size_t at, u32 value) {
    bytes[at] = static_cast<u8>(value >> 24);
    bytes[at + 1] = static_cast<u8>(value >> 16);
    bytes[at + 2] = static_cast<u8>(value >> 8);
    bytes[at + 3] = static_cast<u8>(value);
}

std::vector<u8> frame(bool mono) {
    constexpr u32 samples = 3;
    constexpr u32 blockBytes = 3;
    std::vector<u8> bytes(80 + blockBytes * (mono ? 1 : 2), 0);
    put32(bytes, 0, mono ? 0 : blockBytes);
    put32(bytes, 4, samples);
    bytes[80] = 0;       // predictor 0, scale 0
    bytes[81] = 0x12;    // samples +1, +2
    bytes[82] = 0x30;    // sample +3
    if (!mono) {
        bytes[83] = 0;
        bytes[84] = 0x45; // samples +4, +5
        bytes[85] = 0x60; // sample +6
    }
    return bytes;
}

} // namespace

int main() {
    std::array<s16, 8> out{};
    auto stereo = frame(false);
    assert(NativeThp::decodeAudio(out.data(), stereo.data(), stereo.size(), 4, 0) == 3);
    assert((std::array<s16, 6>{out[0], out[1], out[2], out[3], out[4], out[5]} ==
            std::array<s16, 6>{1, 4, 2, 5, 3, 6}));

    out.fill(0);
    assert(NativeThp::decodeAudio(out.data(), stereo.data(), stereo.size(), 4, 1) == 3);
    assert((std::array<s16, 6>{out[0], out[1], out[2], out[3], out[4], out[5]} ==
            std::array<s16, 6>{4, 5, 6, 1, 2, 3}));

    auto mono = frame(true);
    out.fill(0);
    assert(NativeThp::decodeAudio(out.data(), mono.data(), mono.size(), 4, 0) == 3);
    assert((std::array<s16, 6>{out[0], out[1], out[2], out[3], out[4], out[5]} ==
            std::array<s16, 6>{1, 1, 2, 2, 3, 3}));

    // Negative ADPCM samples must retain signed-nibble and rounding behavior.
    mono[81] = 0xf8;
    out.fill(0);
    assert(NativeThp::decodeAudio(out.data(), mono.data(), mono.size(), 4, 0) == 3);
    assert(out[0] == -1 && out[2] == -8 && out[4] == 3);

    out.fill(77);
    assert(NativeThp::decodeAudio(out.data(), stereo.data(), stereo.size() - 1, 4, 0) == 0);
    assert(out[0] == 77 && out[1] == 77); // malformed input is rejected before output changes
    assert(NativeThp::decodeAudio(out.data(), stereo.data(), stereo.size(), 2, 0) == 0);
    assert(out[0] == 77 && out[1] == 77);
    assert(NativeThp::decodeAudio(out.data(), stereo.data(), 79, 4, 0) == 0);
    assert(NativeThp::decodeAudio(out.data(), stereo.data(), stereo.size(), 4, 7) == 0);
    std::puts("PASS host THP ADPCM stereo/interleaved, stereo/planar, mono, signed samples, bounds and malformed-frame checks");
}
