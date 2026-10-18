#include <Native/ThpAudio.hpp>

#include <algorithm>
#include <cstdint>
#include <limits>

namespace NativeThp {
namespace {

constexpr std::size_t kHeaderBytes = 80;

u16 readBe16(const u8* p) noexcept {
    return static_cast<u16>((u16(p[0]) << 8) | u16(p[1]));
}

u32 readBe32(const u8* p) noexcept {
    return (u32(p[0]) << 24) | (u32(p[1]) << 16) | (u32(p[2]) << 8) | u32(p[3]);
}

s32 signed16(u16 value) noexcept {
    return (value & 0x8000u) ? static_cast<s32>(value) - 0x10000 : static_cast<s32>(value);
}

s32 sampleNibble(const u8* data, u32 index) noexcept {
    const u8 packed = data[1 + index / 2];
    const s32 value = (index & 1u) ? (packed & 0x0f) : (packed >> 4);
    return (value & 8) ? value - 16 : value;
}

s32 floorShift16(s64 value) noexcept {
    if (value >= 0) return static_cast<s32>(value / 65536);
    return static_cast<s32>(-((-value + 65535) / 65536));
}

bool decodeChannel(s16* output, const u8* encoded, u32 count,
                   const s16 coefficients[8][2], s32 yn1, s32 yn2,
                   u32 stride) noexcept {
    const u8 predictorScale = encoded[0];
    const u32 predictor = (predictorScale >> 4) & 7u;
    const u32 scale = predictorScale & 0x0fu;
    const s64 scaleFactor = s64(1) << scale;

    for (u32 i = 0; i < count; ++i) {
        const s32 sample = sampleNibble(encoded, i);
        s64 yn = s64(coefficients[predictor][1]) * yn2 +
                 s64(coefficients[predictor][0]) * yn1 +
                 s64(sample) * scaleFactor * 2048;
        yn = yn * 32 + 0x8000;
        yn = std::clamp<s64>(yn, std::numeric_limits<s32>::min(),
                            std::numeric_limits<s32>::max());
        const s32 decoded = floorShift16(yn);
        output[std::size_t(i) * stride] = static_cast<s16>(decoded);
        yn2 = yn1;
        yn1 = decoded;
    }
    return true;
}

} // namespace

u32 decodeAudio(s16* output, const u8* frame, std::size_t frameBytes,
                u32 outputCapacityFrames, s32 planar) noexcept {
    if (!output || !frame || frameBytes < kHeaderBytes || (planar != 0 && planar != 1)) return 0;

    const u32 rightOffset = readBe32(frame + 0);
    const u32 count = readBe32(frame + 4);
    if (!count || count > outputCapacityFrames) return 0;

    const u64 channelBytes64 = 1u + (u64(count) + 1u) / 2u;
    if (channelBytes64 > std::numeric_limits<std::size_t>::max()) return 0;
    const std::size_t channelBytes = static_cast<std::size_t>(channelBytes64);
    const u8* left = frame + kHeaderBytes;
    const bool mono = rightOffset == 0;
    if (mono) {
        if (channelBytes > frameBytes - kHeaderBytes) return 0;
    } else {
        if (rightOffset < channelBytes || rightOffset > frameBytes - kHeaderBytes ||
            channelBytes > frameBytes - kHeaderBytes - rightOffset) return 0;
    }

    s16 leftCoefficients[8][2]{};
    s16 rightCoefficients[8][2]{};
    for (u32 predictor = 0; predictor < 8; ++predictor) {
        for (u32 coefficient = 0; coefficient < 2; ++coefficient) {
            leftCoefficients[predictor][coefficient] = static_cast<s16>(
                signed16(readBe16(frame + 8 + (predictor * 2 + coefficient) * 2)));
            rightCoefficients[predictor][coefficient] = static_cast<s16>(
                signed16(readBe16(frame + 40 + (predictor * 2 + coefficient) * 2)));
        }
    }
    const s32 leftYn1 = signed16(readBe16(frame + 72));
    const s32 leftYn2 = signed16(readBe16(frame + 74));
    const s32 rightYn1 = signed16(readBe16(frame + 76));
    const s32 rightYn2 = signed16(readBe16(frame + 78));

    if (planar) {
        s16* rightOut = output;
        s16* leftOut = output + count;
        decodeChannel(leftOut, left, count, leftCoefficients, leftYn1, leftYn2, 1);
        if (mono) {
            std::copy_n(leftOut, count, rightOut);
        } else {
            decodeChannel(rightOut, frame + kHeaderBytes + rightOffset, count,
                          rightCoefficients, rightYn1, rightYn2, 1);
        }
    } else {
        decodeChannel(output, left, count, leftCoefficients, leftYn1, leftYn2, 2);
        if (mono) {
            for (u32 i = 0; i < count; ++i) output[2 * i + 1] = output[2 * i];
        } else {
            decodeChannel(output + 1, frame + kHeaderBytes + rightOffset, count,
                          rightCoefficients, rightYn1, rightYn2, 2);
        }
    }
    return count;
}

} // namespace NativeThp
