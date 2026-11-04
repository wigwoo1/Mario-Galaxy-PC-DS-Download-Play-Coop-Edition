#pragma once
#include <cstdint>

namespace Native {
enum class AiOutputBackend { Stopped, WaveOut, SilentClock };
struct AiOutputSnapshot {
    AiOutputBackend backend;
    std::uint32_t sampleRate;
    std::uint32_t queuedBlocks;
    std::uint32_t maximumQueuedBlocks;
    std::uint64_t completedFrames;
    std::uint64_t deviceCompletedFrames;
};
// Read-only diagnostic evidence; does not select or replace the audio backend.
AiOutputSnapshot nativeAiOutputSnapshot() noexcept;
}
