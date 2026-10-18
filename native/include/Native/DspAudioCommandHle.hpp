#pragma once

#include <cstddef>
#include <cstdint>

namespace Native {

// Completes only the JAudio control/frame packets needed to keep the original
// audio task queue moving. This host fallback does not synthesize PCM.
class DspAudioCommandHle {
public:
    enum class Result : std::int32_t {
        NoBackend = 0,
        AcceptedSilent = 1,
        Unsupported = -1,
        Malformed = -2
    };

    Result processBatch(const std::uint32_t* words, std::size_t count,
                        std::uint16_t* completionToken) const noexcept;
};

} // namespace Native
