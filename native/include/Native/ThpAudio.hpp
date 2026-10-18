#pragma once

#include <revolution/thp.h>

#include <cstddef>

namespace NativeThp {

// Decode one big-endian THP ADPCM component into signed 16-bit samples.
// Returns zero without writing output if any declared input/output extent is
// invalid or truncated.
u32 decodeAudio(s16* output, const u8* frame, std::size_t frameBytes,
                u32 outputCapacityFrames, s32 planar) noexcept;

} // namespace NativeThp
