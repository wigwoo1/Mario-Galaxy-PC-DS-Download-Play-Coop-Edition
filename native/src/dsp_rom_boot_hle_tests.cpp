#include <Native/DspRomBootHle.hpp>
#include <Native/DspUcodeImage.hpp>

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <vector>

namespace {
constexpr std::uint32_t kBase = 0x80F3A001u;
constexpr std::uint32_t kImem = 0x80F3C002u;
constexpr std::uint32_t kLength = 0x80F3A002u;
constexpr std::uint32_t kDmem = 0x80F3B002u;
constexpr std::uint32_t kStart = 0x80F3D001u;
constexpr std::array<std::uint32_t, 10> kUpload = {
    kBase, 0, kImem, 0, kLength, 0x1F00, kDmem, 0, kStart, 0};

bool runUpload(Native::DspRomBootHle& hle, std::uint32_t base) {
    for (std::size_t i = 0; i < kUpload.size(); ++i) {
        const auto mail = i == 1 ? base : kUpload[i];
        if (!hle.acceptCpuMail(mail)) return false;
    }
    return true;
}
}

int main() {
    Native::DspAddressSpace addresses;
    const auto* original = Native::originalDspUcodeImage();
    const auto imageBytes = Native::originalDspUcodeImageBytes();
    assert(imageBytes == 0x1F00);
    auto base = addresses.map(original, imageBytes);
    assert(base);

    Native::DspRomBootHle hle(addresses);
    assert(hle.begin(base, imageBytes));
    std::uint32_t mail = 0;
    assert(hle.tryReadDspMail(&mail) && mail == 0x8071FEEDu);
    assert(!hle.tryReadDspMail(&mail));
    assert(runUpload(hle, base));
    assert(hle.state() == Native::DspRomBootHle::State::Running);
    assert(hle.imageHash() == 0xD643001Fu);
    assert(hle.tryReadDspMail(&mail) && mail == 0xDCD10000u);
    assert(hle.tryReadDspMail(&mail) && mail == 0xF3551111u);
    assert(!hle.tryReadDspMail(&mail));

    Native::DspRomBootHle wrongAddress(addresses);
    assert(wrongAddress.begin(base, imageBytes));
    assert(wrongAddress.tryReadDspMail(&mail));
    assert(wrongAddress.acceptCpuMail(kBase));
    assert(!wrongAddress.acceptCpuMail(base + 32));
    assert(wrongAddress.error() == Native::DspRomBootHle::Error::InvalidGuestAddress);

    Native::DspRomBootHle wrongImage(addresses);
    std::vector<std::uint8_t> corrupted(original, original + imageBytes);
    auto badBase = addresses.map(corrupted.data(), corrupted.size());
    assert(badBase);
    assert(wrongImage.begin(badBase, corrupted.size()));
    assert(wrongImage.tryReadDspMail(&mail));
    corrupted[17] ^= 0x80;
    assert(!runUpload(wrongImage, badBase));
    assert(wrongImage.state() == Native::DspRomBootHle::State::Failed);
    assert(wrongImage.error() == Native::DspRomBootHle::Error::UnsupportedUcode);
    assert(!wrongImage.tryReadDspMail(&mail));

    Native::DspRomBootHle outOfRange(addresses);
    assert(!outOfRange.begin(base + 0x100, imageBytes));
    assert(outOfRange.error() == Native::DspRomBootHle::Error::InvalidImage);
    std::puts("PASS DSP ROM task-upload order, bounded image, SMG uCode hash gate and handshake mails");
}
