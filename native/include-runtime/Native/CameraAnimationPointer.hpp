#pragma once
#include <revolution/types.h>

namespace Native {
// CameraParamChunk::mNum1 is an s32 in the original game ABI. Host pointers must
// never be truncated into it; these functions preserve them behind stable handles.
s32 storeCameraAnimationData(const void* data);
u8* resolveCameraAnimationData(s32 handle);
}