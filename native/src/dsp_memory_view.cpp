#include <Native/DspMemoryView.hpp>

#include <array>
#include <limits>

namespace Native {

bool DspMemoryView::readBytes(DspAddressSpace::Address address, void* destination,
                              std::size_t size) const noexcept {
    if (size == 0) return destination != nullptr || address != 0;
    if (!destination) return false;
    const void* source = addressSpace_.resolve(address, size);
    if (!source) return false;
    std::memmove(destination, source, size);
    return true;
}

bool DspMemoryView::writeBytes(DspAddressSpace::Address address, const void* source,
                               std::size_t size) const noexcept {
    if (size == 0) return source != nullptr || address != 0;
    if (!source) return false;
    void* destination = addressSpace_.resolve(address, size);
    if (!destination) return false;
    std::memmove(destination, source, size);
    return true;
}

bool DspMemoryView::readBigEndian16(DspAddressSpace::Address address,
                                   std::uint16_t& value) const noexcept {
    std::array<std::uint8_t, 2> bytes{};
    if (!readBytes(address, bytes.data(), bytes.size())) return false;
    value = static_cast<std::uint16_t>((std::uint16_t(bytes[0]) << 8) | bytes[1]);
    return true;
}

bool DspMemoryView::readBigEndian32(DspAddressSpace::Address address,
                                   std::uint32_t& value) const noexcept {
    std::array<std::uint8_t, 4> bytes{};
    if (!readBytes(address, bytes.data(), bytes.size())) return false;
    value = (std::uint32_t(bytes[0]) << 24) | (std::uint32_t(bytes[1]) << 16) |
            (std::uint32_t(bytes[2]) << 8) | std::uint32_t(bytes[3]);
    return true;
}

bool DspMemoryView::writeBigEndian16(DspAddressSpace::Address address,
                                    std::uint16_t value) const noexcept {
    const std::array<std::uint8_t, 2> bytes{
        static_cast<std::uint8_t>(value >> 8), static_cast<std::uint8_t>(value)};
    return writeBytes(address, bytes.data(), bytes.size());
}

bool DspMemoryView::writeBigEndian32(DspAddressSpace::Address address,
                                    std::uint32_t value) const noexcept {
    const std::array<std::uint8_t, 4> bytes{
        static_cast<std::uint8_t>(value >> 24), static_cast<std::uint8_t>(value >> 16),
        static_cast<std::uint8_t>(value >> 8), static_cast<std::uint8_t>(value)};
    return writeBytes(address, bytes.data(), bytes.size());
}

bool DspMemoryView::copyBigEndianWordsToHost(DspAddressSpace::Address address,
                                             void* destination,
                                             std::size_t byteCount) const noexcept {
    if ((byteCount & 1) != 0 || (byteCount != 0 && !destination)) return false;
    if (byteCount > std::numeric_limits<DspAddressSpace::Address>::max() - address) return false;
    if (byteCount != 0 && !addressSpace_.resolve(address, byteCount)) return false;
    auto* output = static_cast<std::uint8_t*>(destination);
    for (std::size_t offset = 0; offset < byteCount; offset += 2) {
        std::uint16_t value;
        if (!readBigEndian16(address + static_cast<DspAddressSpace::Address>(offset), value))
            return false;
        std::memcpy(output + offset, &value, sizeof(value));
    }
    return true;
}

} // namespace Native
