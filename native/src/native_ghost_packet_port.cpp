#include "Game/Player/GhostPacket.hpp"

#include <cstring>

namespace {
u16 readBigEndian16(const u8* bytes) {
    return static_cast<u16>((static_cast<u16>(bytes[0]) << 8) | bytes[1]);
}

u32 readBigEndian32(const u8* bytes) {
    return (static_cast<u32>(bytes[0]) << 24) |
           (static_cast<u32>(bytes[1]) << 16) |
           (static_cast<u32>(bytes[2]) << 8) |
           static_cast<u32>(bytes[3]);
}
}

GhostPacket::GhostPacket(void* data, u32 len) {
    mDataPtr = static_cast<u8*>(data);
    mCurOffs = 0;
    _C = len;
}

void GhostPacket::read(u8* out, u32 len) {
    if (out == nullptr || mDataPtr == nullptr) return;
    std::memcpy(out, mDataPtr + mCurOffs, len);
    mCurOffs += len;
}

void GhostPacket::read(u32* out) {
    if (out == nullptr || mDataPtr == nullptr) return;
    *out = readBigEndian32(mDataPtr + mCurOffs);
    mCurOffs += 4;
}

void GhostPacket::read(s16* out) {
    if (out == nullptr || mDataPtr == nullptr) return;
    *out = static_cast<s16>(readBigEndian16(mDataPtr + mCurOffs));
    mCurOffs += 2;
}

void GhostPacket::read(char** out) {
    if (out == nullptr || mDataPtr == nullptr) return;
    char* value = reinterpret_cast<char*>(mDataPtr + mCurOffs);
    *out = value;
    mCurOffs += static_cast<u32>(std::strlen(value) + 1);
}

void GhostPacket::read(s8* out) {
    if (out == nullptr || mDataPtr == nullptr) return;
    *out = static_cast<s8>(mDataPtr[mCurOffs]);
    ++mCurOffs;
}

void GhostPacket::read(TVec3Sc* out) {
    if (out == nullptr || mDataPtr == nullptr) return;
    out->x = static_cast<s8>(mDataPtr[mCurOffs++]);
    out->y = static_cast<s8>(mDataPtr[mCurOffs++]);
    out->z = static_cast<s8>(mDataPtr[mCurOffs++]);
}

void GhostPacket::read(TVec3s* out) {
    if (out == nullptr || mDataPtr == nullptr) return;
    out->x = static_cast<s16>(readBigEndian16(mDataPtr + mCurOffs));
    mCurOffs += 2;
    out->y = static_cast<s16>(readBigEndian16(mDataPtr + mCurOffs));
    mCurOffs += 2;
    out->z = static_cast<s16>(readBigEndian16(mDataPtr + mCurOffs));
    mCurOffs += 2;
}
