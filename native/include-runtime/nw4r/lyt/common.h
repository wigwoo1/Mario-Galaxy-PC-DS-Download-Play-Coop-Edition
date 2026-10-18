#pragma once

// Petari targets Wii's big-endian CPU, so its inline four-byte signature
// helper interprets file bytes using host byte order. Preserve the upstream
// header declarations while renaming that helper, then provide the file-order
// version used by little-endian native builds.
#define GetSignatureInt GetSignatureInt_PetariBigEndian
#define TestFileVersion TestFileVersion_PetariBigEndian
#include_next "nw4r/lyt/common.h"
#undef GetSignatureInt
#undef TestFileVersion

namespace nw4r {
namespace lyt {
namespace detail {
inline s32 GetSignatureInt(const char sig[4]) {
    return (s32(u8(sig[0])) << 24) | (s32(u8(sig[1])) << 16) |
           (s32(u8(sig[2])) << 8) | s32(u8(sig[3]));
}
inline u16 ReadBigEndianU16(const u8* bytes) {
    return u16((u16(bytes[0]) << 8) | u16(bytes[1]));
}
inline u32 ReadBigEndianU32(const u8* bytes) {
    return (u32(bytes[0]) << 24) | (u32(bytes[1]) << 16) |
           (u32(bytes[2]) << 8) | u32(bytes[3]);
}
inline f32 ReadBigEndianF32(const u8* bytes) {
    union { u32 bits; f32 value; } converted{};
    converted.bits = ReadBigEndianU32(bytes);
    return converted.value;
}
inline bool TestFileVersion(const res::BinaryFileHeader& fileHeader) {
    const u8* bytes = reinterpret_cast<const u8*>(&fileHeader);
    const u16 version = ReadBigEndianU16(bytes + 6);
    return (version >> 8) == 0 && (version & 0xff) >= 9;
}
} // namespace detail
} // namespace lyt
} // namespace nw4r
