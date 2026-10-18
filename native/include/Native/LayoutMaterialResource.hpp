#pragma once
#include <nw4r/lyt/common.h>
#include <nw4r/lyt/resources.h>
#include <nw4r/lyt/types.h>
#include <cstddef>

namespace Native {
// WindowContent begins with colors, not a DataBlockHeader. Its material
// index is the big-endian field at byte 16, also for untextured windows.
inline u16 readLayoutWindowMaterialIndex(const nw4r::lyt::res::WindowContent& source) {
    return nw4r::lyt::detail::ReadBigEndianU16(
        reinterpret_cast<const u8*>(&source) + offsetof(nw4r::lyt::res::WindowContent, materialIdx));
}
inline nw4r::lyt::res::MaterialResourceNum readLayoutMaterialCounts(
    const nw4r::lyt::res::MaterialResourceNum& source) {
    nw4r::lyt::res::MaterialResourceNum result{};
    result.bits = nw4r::lyt::detail::ReadBigEndianU32(reinterpret_cast<const u8*>(&source.bits));
    return result;
}
inline GXColorS10 readLayoutTevColor(const GXColorS10& source) {
    const auto* bytes = reinterpret_cast<const u8*>(&source);
    return {s16(nw4r::lyt::detail::ReadBigEndianU16(bytes + offsetof(GXColorS10, r))),
            s16(nw4r::lyt::detail::ReadBigEndianU16(bytes + offsetof(GXColorS10, g))),
            s16(nw4r::lyt::detail::ReadBigEndianU16(bytes + offsetof(GXColorS10, b))),
            s16(nw4r::lyt::detail::ReadBigEndianU16(bytes + offsetof(GXColorS10, a)))};
}
inline nw4r::lyt::TexSRT readLayoutTextureSrt(const nw4r::lyt::TexSRT& source) {
    using nw4r::lyt::detail::ReadBigEndianF32;
    const auto* bytes = reinterpret_cast<const u8*>(&source);
    nw4r::lyt::TexSRT result{};
    result.translate.x = ReadBigEndianF32(bytes + offsetof(nw4r::lyt::TexSRT, translate));
    result.translate.y = ReadBigEndianF32(bytes + offsetof(nw4r::lyt::TexSRT, translate) + 4);
    result.rotate = ReadBigEndianF32(bytes + offsetof(nw4r::lyt::TexSRT, rotate));
    result.scale.x = ReadBigEndianF32(bytes + offsetof(nw4r::lyt::TexSRT, scale));
    result.scale.y = ReadBigEndianF32(bytes + offsetof(nw4r::lyt::TexSRT, scale) + 4);
    return result;
}
}
