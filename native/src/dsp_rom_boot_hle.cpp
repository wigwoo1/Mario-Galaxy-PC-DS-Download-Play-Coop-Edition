#include <Native/DspRomBootHle.hpp>

#include <limits>

namespace Native {
namespace {
constexpr std::uint32_t kRomReady = 0x8071FEEDu;
constexpr std::uint32_t kUploadAddressCommand = 0x80F3A001u;
constexpr std::uint32_t kSetImemAddressCommand = 0x80F3C002u;
constexpr std::uint32_t kSetLengthCommand = 0x80F3A002u;
constexpr std::uint32_t kSetDmemLengthCommand = 0x80F3B002u;
constexpr std::uint32_t kStartCommand = 0x80F3D001u;
constexpr std::uint32_t kDspInit = 0xDCD10000u;
constexpr std::uint32_t kZeldaHandshake = 0xF3551111u;
constexpr std::uint32_t kSmgUcodeHash = 0xD643001Fu;
constexpr std::size_t kSmgUcodeLength = 0x1F00;
}

DspRomBootHle::DspRomBootHle(const DspAddressSpace& addressSpace) noexcept
    : addressSpace_(addressSpace) {}

bool DspRomBootHle::begin(std::uint32_t guestImageAddress,
                          std::size_t imageBytes) noexcept {
    state_ = State::Idle;
    error_ = Error::None;
    step_ = UploadStep::UcodeAddressCommand;
    guestImageAddress_ = 0;
    imageBytes_ = 0;
    imageHash_ = 0;
    dspMails_.clear();

    if (!guestImageAddress || imageBytes != kSmgUcodeLength ||
        !addressSpace_.resolve(guestImageAddress, imageBytes))
        return fail(Error::InvalidImage);

    guestImageAddress_ = guestImageAddress;
    imageBytes_ = imageBytes;
    state_ = State::Uploading;
    dspMails_.push_back(kRomReady);
    return true;
}

bool DspRomBootHle::acceptCpuMail(std::uint32_t mail) noexcept {
    if (state_ != State::Uploading) return fail(Error::UnexpectedMail);

    switch (step_) {
    case UploadStep::UcodeAddressCommand:
        if (mail != kUploadAddressCommand) return fail(Error::UnexpectedMail);
        step_ = UploadStep::UcodeAddress;
        return true;
    case UploadStep::UcodeAddress:
        if (mail != guestImageAddress_) return fail(Error::InvalidGuestAddress);
        step_ = UploadStep::ImemCommand;
        return true;
    case UploadStep::ImemCommand:
        if (mail != kSetImemAddressCommand) return fail(Error::UnexpectedMail);
        step_ = UploadStep::ImemAddress;
        return true;
    case UploadStep::ImemAddress:
        if ((mail & 0xFFFFu) != 0) return fail(Error::InvalidTaskLayout);
        step_ = UploadStep::UcodeLengthCommand;
        return true;
    case UploadStep::UcodeLengthCommand:
        if (mail != kSetLengthCommand) return fail(Error::UnexpectedMail);
        step_ = UploadStep::UcodeLength;
        return true;
    case UploadStep::UcodeLength:
        if ((mail & 0xFFFFu) != imageBytes_) return fail(Error::InvalidTaskLayout);
        step_ = UploadStep::DmemCommand;
        return true;
    case UploadStep::DmemCommand:
        if (mail != kSetDmemLengthCommand) return fail(Error::UnexpectedMail);
        step_ = UploadStep::DmemLength;
        return true;
    case UploadStep::DmemLength:
        if ((mail & 0xFFFFu) != 0) return fail(Error::InvalidTaskLayout);
        step_ = UploadStep::StartCommand;
        return true;
    case UploadStep::StartCommand:
        if (mail != kStartCommand) return fail(Error::UnexpectedMail);
        step_ = UploadStep::StartVector;
        return true;
    case UploadStep::StartVector: {
        if ((mail & 0xFFFFu) != 0) return fail(Error::InvalidTaskLayout);
        const auto* image = addressSpace_.resolve(guestImageAddress_, imageBytes_);
        if (!image) return fail(Error::InvalidGuestAddress);
        imageHash_ = hashEctorBigEndian(image, imageBytes_);
        if (imageHash_ != kSmgUcodeHash) return fail(Error::UnsupportedUcode);
        state_ = State::Running;
        dspMails_.push_back(kDspInit);
        dspMails_.push_back(kZeldaHandshake);
        return true;
    }
    }
    return fail(Error::UnexpectedMail);
}

bool DspRomBootHle::tryReadDspMail(std::uint32_t* mail) noexcept {
    if (!mail || dspMails_.empty()) return false;
    *mail = dspMails_.front();
    dspMails_.pop_front();
    return true;
}

std::uint32_t DspRomBootHle::hashEctorBigEndian(const void* bytes,
                                                std::size_t length) noexcept {
    if (!bytes && length) return 0;
    const auto* data = static_cast<const std::uint8_t*>(bytes);
    std::uint32_t hash = 0;
    for (std::size_t i = 0; i < length; ++i) {
        hash ^= data[i];
        hash = (hash << 3) | (hash >> 29);
    }
    return hash;
}

bool DspRomBootHle::fail(Error error) noexcept {
    state_ = State::Failed;
    error_ = error;
    dspMails_.clear();
    return false;
}

} // namespace Native
