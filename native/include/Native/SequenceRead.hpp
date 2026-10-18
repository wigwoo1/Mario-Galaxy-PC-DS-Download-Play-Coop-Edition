#pragma once
#include <revolution/types.h>
#include <cstdint>
#include <limits>
#include <stdexcept>
namespace NativeSequence {
inline u32 offset(const u8* base,const u8* cursor){const auto b=reinterpret_cast<std::uintptr_t>(base),c=reinterpret_cast<std::uintptr_t>(cursor);if(c<b||c-b>std::numeric_limits<u32>::max())throw std::out_of_range("Native sequence offset exceeds u32 domain");return static_cast<u32>(c-b);}
inline u16 read16(const u8* p){return static_cast<u16>((u16(p[0])<<8)|u16(p[1]));}
inline u32 read24(const u8* p){return (u32(p[0])<<16)|(u32(p[1])<<8)|u32(p[2]);}
inline u32 read32(const u8* p){return (u32(p[0])<<24)|(u32(p[1])<<16)|(u32(p[2])<<8)|u32(p[3]);}
}
