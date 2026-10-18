#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <mutex>

namespace Native {

// Gives Wii DSP commands stable 32-bit addresses for host buffers. The DSP
// protocol stores 32-bit RAM addresses, while this port runs with 64-bit host
// pointers; callers must translate through this table instead of truncating.
class DspAddressSpace {
public:
    using Address = std::uint32_t;

    // Register a non-empty host memory range and return its guest address.
    // Re-registering a contained range returns the existing address plus the
    // byte offset. Returns zero for null, empty, or unrepresentable ranges.
    Address map(const void* memory, std::size_t size) noexcept;

    // Translate a pointer already covered by a mapping. Extent may be zero to
    // represent the one-past-end address used by frame-buffer commands.
    Address addressOf(const void* memory, std::size_t extent = 0) const noexcept;

    // Resolve an address only when the entire requested span belongs to one
    // registered range. A zero-sized span may point at the range's end.
    void* resolve(Address address, std::size_t size = 0) const noexcept;

    bool unmap(Address base) noexcept;
    void clear() noexcept;
    std::size_t mappingCount() const noexcept;

private:
    struct Region {
        std::uintptr_t hostBase;
        std::size_t size;
    };

    mutable std::mutex mutex_;
    std::map<Address, Region> regions_;
    Address nextAddress_ = 0x10000000u;
};

// Shared registry used by native JAudio/DSP providers so command producers
// and a future DSP consumer resolve the same guest-address mappings.
DspAddressSpace& nativeDspAddressSpace() noexcept;

} // namespace Native
