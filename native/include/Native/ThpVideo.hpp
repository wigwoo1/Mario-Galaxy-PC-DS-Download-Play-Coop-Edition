#pragma once
#include <cstddef>
#include <revolution/types.h>

namespace NativeThp {
class VideoFrameScope {
public:
    VideoFrameScope(const void* bytes, std::size_t byteCount, u32 width, u32 height) noexcept;
    ~VideoFrameScope();
    VideoFrameScope(const VideoFrameScope&) = delete;
    VideoFrameScope& operator=(const VideoFrameScope&) = delete;

private:
    const void* mPrevious;
    const void* mContext;
    alignas(std::max_align_t) u8 mStorage[32];
};

bool videoDecoderInitialized() noexcept;
}
