#pragma once

#include <Native/DspAddressSpace.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace Native {

// Bounded byte access for DSP-visible memory. Callers choose host-native or
// guest big-endian interpretation explicitly; JAS host structures and retail
// sample data do not share one universal byte-order contract.
class DspMemoryView {
public:
    explicit DspMemoryView(const DspAddressSpace& addressSpace) noexcept
        : addressSpace_(addressSpace) {}

    bool readBytes(DspAddressSpace::Address address, void* destination,
                   std::size_t size) const noexcept;
    bool writeBytes(DspAddressSpace::Address address, const void* source,
                    std::size_t size) const noexcept;

    bool readBigEndian16(DspAddressSpace::Address address, std::uint16_t& value) const noexcept;
    bool readBigEndian32(DspAddressSpace::Address address, std::uint32_t& value) const noexcept;
    bool writeBigEndian16(DspAddressSpace::Address address, std::uint16_t value) const noexcept;
    bool writeBigEndian32(DspAddressSpace::Address address, std::uint32_t value) const noexcept;

    template <typename T>
    bool readHost(DspAddressSpace::Address address, T& value) const noexcept {
        static_assert(std::is_trivially_copyable_v<T>);
        return readBytes(address, &value, sizeof(T));
    }

    template <typename T>
    bool writeHost(DspAddressSpace::Address address, const T& value) const noexcept {
        static_assert(std::is_trivially_copyable_v<T>);
        return writeBytes(address, &value, sizeof(T));
    }

    // Copies big-endian 16-bit samples/words into host-native 16-bit storage.
    bool copyBigEndianWordsToHost(DspAddressSpace::Address address, void* destination,
                                  std::size_t byteCount) const noexcept;

private:
    const DspAddressSpace& addressSpace_;
};

} // namespace Native
