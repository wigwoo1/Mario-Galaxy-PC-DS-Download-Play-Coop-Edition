#include <Native/CameraAnimationPointer.hpp>
#include <cassert>
#include <thread>
#include <vector>

int main() {
    assert(Native::storeCameraAnimationData(nullptr) == 0);
    assert(Native::resolveCameraAnimationData(0) == nullptr);
    assert(Native::resolveCameraAnimationData(-1) == nullptr);
    assert(Native::resolveCameraAnimationData(0x7ffffffe) == nullptr);

    u8 first[4] = {1, 2, 3, 4};
    u8 second[4] = {5, 6, 7, 8};
    const s32 firstHandle = Native::storeCameraAnimationData(first);
    const s32 secondHandle = Native::storeCameraAnimationData(second);
    assert(firstHandle > 0 && secondHandle > 0 && firstHandle != secondHandle);
    assert(Native::storeCameraAnimationData(first) == firstHandle);
    assert(Native::resolveCameraAnimationData(firstHandle) == first);
    assert(Native::resolveCameraAnimationData(secondHandle) == second);

    std::vector<s32> handles(8);
    std::vector<std::thread> threads;
    for (size_t i = 0; i < handles.size(); ++i) {
        threads.emplace_back([&, i] { handles[i] = Native::storeCameraAnimationData(first); });
    }
    for (auto& thread : threads) thread.join();
    for (const s32 handle : handles) assert(handle == firstHandle);
}