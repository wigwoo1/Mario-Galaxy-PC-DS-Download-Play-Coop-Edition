#include <Native/CameraAnimationPointer.hpp>
#include <cstdint>
#include <limits>
#include <mutex>
#include <unordered_map>

namespace {
std::mutex gCameraAnimationPointerMutex;
std::unordered_map<const void*, s32> gCameraAnimationPointerHandles;
std::unordered_map<s32, const void*> gCameraAnimationPointers;
s32 gNextCameraAnimationHandle = 1;
}

namespace Native {
s32 storeCameraAnimationData(const void* data) {
    if (data == nullptr) return 0;
    std::lock_guard<std::mutex> lock(gCameraAnimationPointerMutex);
    const auto existing = gCameraAnimationPointerHandles.find(data);
    if (existing != gCameraAnimationPointerHandles.end()) return existing->second;

    // Zero remains the original null/default value. Exhaustion fails closed.
    while (gNextCameraAnimationHandle > 0 && (gCameraAnimationPointers.find(gNextCameraAnimationHandle) != gCameraAnimationPointers.end())) {
        if (gNextCameraAnimationHandle == std::numeric_limits<s32>::max()) return 0;
        ++gNextCameraAnimationHandle;
    }
    if (gNextCameraAnimationHandle <= 0) return 0;
    const s32 handle = gNextCameraAnimationHandle++;
    gCameraAnimationPointerHandles.emplace(data, handle);
    gCameraAnimationPointers.emplace(handle, data);
    return handle;
}

u8* resolveCameraAnimationData(s32 handle) {
    if (handle <= 0) return nullptr;
    std::lock_guard<std::mutex> lock(gCameraAnimationPointerMutex);
    const auto found = gCameraAnimationPointers.find(handle);
    return found == gCameraAnimationPointers.end() ? nullptr : const_cast<u8*>(static_cast<const u8*>(found->second));
}
}