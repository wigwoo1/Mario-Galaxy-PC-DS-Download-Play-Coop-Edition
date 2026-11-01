#include <Native/DspAudioCommandHle.hpp>

namespace Native {

DspAudioCommandHle::Validation DspAudioCommandHle::decodeBatch(
    const std::uint32_t* words,std::size_t count,Command& output) noexcept {
    if (!words) return Validation::Malformed;

    // DSPSendCommands2 represents a one-word command with a zero count.
    const std::size_t wordCount = count == 0 ? 1 : count;
    const std::uint32_t header = words[0];
    const std::uint16_t opcode = static_cast<std::uint16_t>(header >> 16);
    Command next;
    next.completionToken=opcode;

    switch (header & 0xFF000000u) {
    case 0x8E000000u: // Set VARAM base: ARAM is not a host resource.
        if (header != 0x8E000000u || wordCount != 2) return Validation::Malformed;
        next.kind=Kind::Varam;
        next.arguments[0]=words[1];
        break;
    case 0x81000000u: // Configure the original 64-channel DSP tables.
        if (header != 0x81000040u || wordCount != 5) return Validation::Malformed;
        next.kind=Kind::Tables;
        for(unsigned i=0;i<4;++i)next.arguments[i]=words[i+1];
        break;
    case 0x82000000u: // Per-frame mix request; accepted as silence for now.
        if (wordCount != 5) return Validation::Malformed;
        next.kind=Kind::Frame;
        next.subframes=static_cast<std::uint8_t>(header>>16);
        next.mixerLevel=static_cast<std::uint16_t>(header);
        for(unsigned i=0;i<4;++i)next.arguments[i]=words[i+1];
        break;
    default:
        // DSPReleaseHalt2: (group << 16) | active map for 16 of 64 channels.
        // updateAll sends groups 0..3, including zero when all are inactive.
        if (opcode > 3) return Validation::Unsupported;
        if (wordCount != 1) return Validation::Malformed;
        next.kind=Kind::ChannelGroup;
        next.group=static_cast<std::uint8_t>(opcode);
        next.activeChannels=static_cast<std::uint16_t>(header);
        break;
    }

    output=next;
    return Validation::Valid;
}

DspAudioCommandHle::Result DspAudioCommandHle::processBatch(
    const std::uint32_t* words,std::size_t count,
    std::uint16_t* completionToken) const noexcept {
    if(!completionToken)return Result::Malformed;
    Command command;
    switch(decodeBatch(words,count,command)){
    case Validation::Malformed:return Result::Malformed;
    case Validation::Unsupported:return Result::Unsupported;
    case Validation::Valid:break;
    }
    *completionToken=command.completionToken;
    return Result::AcceptedSilent;
}

} // namespace Native
