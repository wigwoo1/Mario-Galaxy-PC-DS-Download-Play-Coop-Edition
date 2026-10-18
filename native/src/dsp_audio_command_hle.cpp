#include <Native/DspAudioCommandHle.hpp>

namespace Native {

DspAudioCommandHle::Result DspAudioCommandHle::processBatch(
    const std::uint32_t* words, std::size_t count,
    std::uint16_t* completionToken) const noexcept {
    if (!words || !completionToken) return Result::Malformed;

    // DSPSendCommands2 represents a one-word command with a zero count.
    const std::size_t wordCount = count == 0 ? 1 : count;
    const std::uint32_t header = words[0];
    const std::uint16_t opcode = static_cast<std::uint16_t>(header >> 16);

    switch (header & 0xFF000000u) {
    case 0x8E000000u: // Set VARAM base: ARAM is not a host resource.
        if (header != 0x8E000000u || wordCount != 2) return Result::Malformed;
        break;
    case 0x81000000u: // Configure the original 64-channel DSP tables.
        if (header != 0x81000040u || wordCount != 5) return Result::Malformed;
        break;
    case 0x82000000u: // Per-frame mix request; accepted as silence for now.
        if (wordCount != 5) return Result::Malformed;
        break;
    default:
        // DSPReleaseHalt2 sends a single packed 0x0055xxxx command.
        if (opcode != 0x0055u || wordCount != 1) return Result::Unsupported;
        break;
    }

    *completionToken = opcode;
    return Result::AcceptedSilent;
}

} // namespace Native
