#pragma once
// Host ABI shim. Never overlay native structs on big-endian disc data.
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cmath>
#include <cfloat>
#include <limits>
using s8 = std::int8_t; using s16 = std::int16_t; using s32 = std::int32_t; using s64 = std::int64_t;
using u8 = std::uint8_t; using u16 = std::uint16_t; using u32 = std::uint32_t; using u64 = std::uint64_t;
using f32 = float; using f64 = double; using vu8=volatile u8; using vu16=volatile u16; using vu32=volatile u32; using vu64=volatile u64; using vs8=volatile s8; using vs16=volatile s16; using vs32=volatile s32; using vs64=volatile s64; using vf32=volatile f32; using vf64=volatile f64;
using BOOL = int;
#define ATTRIBUTE_WEAK __attribute__((weak))
#define ATTRIBUTE_ALIGN(num) __attribute__((aligned(num)))
#define __REGISTER
// The pinned SDK uses __memcpy for non-overlapping object copies with nonnegative sizes.
inline void* __memcpy(void* dst, const void* src, int count) { return std::memcpy(dst, src, static_cast<std::size_t>(count)); }
#ifndef NO_INLINE
#define NO_INLINE __attribute__((noinline))
#endif
static_assert(sizeof(s32) == 4 && sizeof(u64) == 8);



// Native scalar adaptations of compiler intrinsics. __frsqrte uses host sqrt,
// not the PPC estimate table; callers retain their original refinement steps.
inline f32 __frsqrte(f32 value) { return 1.0f / std::sqrt(value); }
inline f64 __fabs(f64 value) { return std::fabs(value); }
inline f32 __fabsf(f32 value) { return std::fabs(value); }
inline s32 __abs(s32 value) {
    // PPC signed negate wraps at INT_MIN; avoid host signed-overflow UB.
    return value == std::numeric_limits<s32>::min() ? value : (value < 0 ? -value : value);
}

// Accept actual fixed arrays only; do not silently count pointer sizes.
template<class T,std::size_t N> constexpr std::size_t nativeArrayCount(const T (&)[N]) noexcept { return N; }
template<class T> constexpr auto nativeArrayCount(const T& value) noexcept(noexcept(value.size())) -> decltype(value.size()) { return value.size(); }
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) nativeArrayCount(array)
#endif
#ifndef ARRAY_SIZEU
#define ARRAY_SIZEU(array) (sizeof(array) / sizeof((array)[0]))
#endif
#ifndef ALIGN_PREV
#define ALIGN_PREV(X, N) ((X) & ~((N) - 1))
#endif
#ifndef ALIGN_NEXT
#define ALIGN_NEXT(X, N) ALIGN_PREV(((X) + (N) - 1), N)
#endif
