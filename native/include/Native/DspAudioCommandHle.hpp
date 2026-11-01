#pragma once

#include <cstddef>
#include <cstdint>
#include <array>

namespace Native {

// Legacy packet fallback: acknowledges supported shapes as explicit silence.
// It does not render PCM, advance channels or produce subframe-completion mail.
// The bounded decoder exposes packet data independently for a real executor.
class DspAudioCommandHle {
public:
    enum class Result : std::int32_t {
        NoBackend = 0,
        AcceptedSilent = 1,
        AcceptedRendered = 2, // Real backend owns deferred completion delivery.
        Unsupported = -1,
        Malformed = -2
    };

    enum class Kind { Varam, Tables, Frame, ChannelGroup };
    enum class Validation { Valid, Unsupported, Malformed };
    struct Command {
        Kind kind=Kind::Varam;
        std::uint16_t completionToken=0;
        std::array<std::uint32_t,4> arguments{};
        std::uint8_t subframes=0;
        std::uint16_t mixerLevel=0;
        std::uint8_t group=0;
        std::uint16_t activeChannels=0;
    };
    // Rejected input leaves output unchanged. Validation is not execution.
    static Validation decodeBatch(const std::uint32_t* words,std::size_t count,
                                  Command& output) noexcept;

    Result processBatch(const std::uint32_t* words, std::size_t count,
                        std::uint16_t* completionToken) const noexcept;
};

} // namespace Native
