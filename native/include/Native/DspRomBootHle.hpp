#pragma once

#include <Native/DspAddressSpace.hpp>

#include <cstddef>
#include <cstdint>
#include <deque>

namespace Native {

// Models only the Wii DSP ROM's task-upload handshake. It validates the
// mapped uCode image before announcing task startup; it does not emulate the
// uploaded Zelda uCode or acknowledge render commands.
class DspRomBootHle {
public:
    enum class State : std::uint8_t { Idle, Uploading, Running, Failed };
    enum class Error : std::uint8_t {
        None, InvalidImage, UnexpectedMail, InvalidGuestAddress,
        InvalidTaskLayout, UnsupportedUcode
    };

    explicit DspRomBootHle(const DspAddressSpace& addressSpace) noexcept;

    // Begin a ROM boot session. guestImageAddress must name the whole image in
    // the same bounded address map used by JAudio/DSP command producers.
    bool begin(std::uint32_t guestImageAddress, std::size_t imageBytes) noexcept;
    // Consume one CPU-to-DSP mailbox word. Returns false and enters Failed on
    // malformed ordering, invalid memory, or an unsupported uCode image.
    bool acceptCpuMail(std::uint32_t mail) noexcept;
    bool tryReadDspMail(std::uint32_t* mail) noexcept;

    State state() const noexcept { return state_; }
    Error error() const noexcept { return error_; }
    std::uint32_t imageHash() const noexcept { return imageHash_; }
    std::uint32_t guestImageAddress() const noexcept { return guestImageAddress_; }
    std::size_t imageBytes() const noexcept { return imageBytes_; }

    static std::uint32_t hashEctorBigEndian(const void* bytes,
                                             std::size_t length) noexcept;

private:
    enum class UploadStep : std::uint8_t {
        UcodeAddressCommand, UcodeAddress, ImemCommand, ImemAddress,
        UcodeLengthCommand, UcodeLength, DmemCommand, DmemLength,
        StartCommand, StartVector
    };

    bool fail(Error error) noexcept;

    const DspAddressSpace& addressSpace_;
    State state_ = State::Idle;
    Error error_ = Error::None;
    UploadStep step_ = UploadStep::UcodeAddressCommand;
    std::uint32_t guestImageAddress_ = 0;
    std::size_t imageBytes_ = 0;
    std::uint32_t imageHash_ = 0;
    std::deque<std::uint32_t> dspMails_;
};

} // namespace Native
