#include <Native/DspAddressSpace.hpp>

#include <limits>

namespace Native {

DspAddressSpace& nativeDspAddressSpace() noexcept {
    static DspAddressSpace addressSpace;
    return addressSpace;
}

namespace {
constexpr DspAddressSpace::Address kFirstAddress = 0x10000000u;
constexpr DspAddressSpace::Address kAddressLimit = 0xf0000000u;
constexpr std::uintptr_t kAlignment = 32;

bool contains(std::uintptr_t base, std::size_t size, std::uintptr_t pointer,
              std::size_t extent) noexcept {
    if (pointer < base) return false;
    const std::uintptr_t offset = pointer - base;
    return offset <= size && extent <= size - static_cast<std::size_t>(offset);
}

} // namespace

DspAddressSpace::Address DspAddressSpace::map(const void* memory,
                                              std::size_t size) noexcept {
    if (!memory || size == 0) return 0;
    const auto host = reinterpret_cast<std::uintptr_t>(memory);
    if (size > std::numeric_limits<std::uintptr_t>::max() - host) return 0;
    if (size >= kAddressLimit) return 0;

    std::lock_guard lock(mutex_);
    for (const auto& [base, region] : regions_) {
        if (contains(region.hostBase, region.size, host, size)) {
            const auto offset = host - region.hostBase;
            if (offset > std::numeric_limits<Address>::max() - base) return 0;
            return base + static_cast<Address>(offset);
        }
        const auto regionEnd = region.hostBase + region.size;
        const auto hostEnd = host + size;
        if (host < regionEnd && region.hostBase < hostEnd) return 0;
    }

    const auto aligned = (static_cast<std::uint64_t>(nextAddress_) +
                          (kAlignment - 1)) & ~(std::uint64_t(kAlignment - 1));
    const auto paddedSize = (static_cast<std::uint64_t>(size) +
                             (kAlignment - 1)) & ~(std::uint64_t(kAlignment - 1));
    if (aligned >= kAddressLimit || paddedSize + kAlignment > kAddressLimit - aligned) return 0;

    const auto base = static_cast<Address>(aligned);
    regions_.emplace(base, Region{host, size});
    // Keep a full guard unit after each range so the previous region's
    // one-past-end pointer cannot alias the next region's base address.
    nextAddress_ = static_cast<Address>(aligned + paddedSize + kAlignment);
    return base;
}

DspAddressSpace::Address DspAddressSpace::addressOf(const void* memory,
                                                    std::size_t extent) const noexcept {
    if (!memory) return 0;
    const auto host = reinterpret_cast<std::uintptr_t>(memory);
    std::lock_guard lock(mutex_);
    for (const auto& [base, region] : regions_) {
        if (!contains(region.hostBase, region.size, host, extent)) continue;
        const auto offset = host - region.hostBase;
        if (offset > std::numeric_limits<Address>::max() - base) return 0;
        return base + static_cast<Address>(offset);
    }
    return 0;
}

void* DspAddressSpace::resolve(Address address, std::size_t size) const noexcept {
    if (address == 0) return nullptr;
    std::lock_guard lock(mutex_);
    auto it = regions_.upper_bound(address);
    if (it == regions_.begin()) return nullptr;
    --it;
    const auto offset = static_cast<std::size_t>(address - it->first);
    if (offset > it->second.size || size > it->second.size - offset) return nullptr;
    return reinterpret_cast<void*>(it->second.hostBase + offset);
}

bool DspAddressSpace::unmap(Address base) noexcept {
    std::lock_guard lock(mutex_);
    return regions_.erase(base) != 0;
}

void DspAddressSpace::clear() noexcept {
    std::lock_guard lock(mutex_);
    regions_.clear();
    nextAddress_ = kFirstAddress;
}

std::size_t DspAddressSpace::mappingCount() const noexcept {
    std::lock_guard lock(mutex_);
    return regions_.size();
}

} // namespace Native
