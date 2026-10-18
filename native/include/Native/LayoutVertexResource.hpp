#pragma once
#include <nw4r/lyt/common.h>

namespace Native {
// BRLYT colors are packed RGBA words; UV components are Wii big-endian floats.
inline nw4r::ut::Color readLayoutVertexColor(const u32& source) {
    return nw4r::ut::Color(nw4r::lyt::detail::ReadBigEndianU32(reinterpret_cast<const u8*>(&source)));
}
inline nw4r::math::VEC2 readLayoutTexCoord(const nw4r::math::VEC2& source) {
    const auto* bytes = reinterpret_cast<const u8*>(&source);
    return nw4r::math::VEC2(nw4r::lyt::detail::ReadBigEndianF32(bytes),
                            nw4r::lyt::detail::ReadBigEndianF32(bytes + sizeof(f32)));
}
}
