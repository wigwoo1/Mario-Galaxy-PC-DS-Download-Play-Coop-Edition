#include <Native/DspAddressSpace.hpp>

#include <array>
#include <cassert>
#include <cstdio>
#include <thread>
#include <vector>

int main() {
    Native::DspAddressSpace space;
    alignas(32) std::array<std::uint8_t, 256> memory{};
    assert(space.map(nullptr, memory.size()) == 0);
    assert(space.map(memory.data(), 0) == 0);

    const auto base = space.map(memory.data(), memory.size());
    assert(base == 0x10000000u);
    assert((base & 31u) == 0);
    assert(space.mappingCount() == 1);
    assert(space.map(memory.data() + 64, 32) == base + 64);
    assert(space.addressOf(memory.data() + 128, 16) == base + 128);
    assert(space.addressOf(memory.data() + memory.size()) == base + memory.size());
    assert(space.addressOf(memory.data() + memory.size(), 1) == 0);
    assert(space.map(memory.data() + memory.size() - 8, 16) == 0);

    assert(space.resolve(base) == memory.data());
    assert(space.resolve(base + 12, 4) == memory.data() + 12);
    assert(space.resolve(base + memory.size()) == memory.data() + memory.size());
    assert(space.resolve(base + memory.size(), 1) == nullptr);
    assert(space.resolve(base - 1) == nullptr);
    assert(space.resolve(0) == nullptr);

    std::array<std::uint8_t, 8> small{};
    const auto second = space.map(small.data(), small.size());
    assert(second != 0 && second > base);
    assert(space.resolve(second, small.size()) == small.data());
    assert(space.unmap(second));
    assert(!space.unmap(second));
    assert(space.resolve(second) == nullptr);

    // Concurrent registration of the same range must produce one stable map.
    space.clear();
    std::array<Native::DspAddressSpace::Address, 16> concurrent{};
    std::vector<std::thread> threads;
    for (std::size_t i = 0; i < concurrent.size(); ++i) {
        threads.emplace_back([&, i] { concurrent[i] = space.map(memory.data(), memory.size()); });
    }
    for (auto& thread : threads) thread.join();
    for (auto address : concurrent) assert(address == 0x10000000u);
    assert(space.mappingCount() == 1);

    auto& sharedA = Native::nativeDspAddressSpace();
    auto& sharedB = Native::nativeDspAddressSpace();
    assert(&sharedA == &sharedB);
    sharedA.clear();
    const auto sharedBase = sharedA.map(memory.data(), memory.size());
    assert(sharedBase && sharedB.addressOf(memory.data() + 16, 4) == sharedBase + 16);

    std::puts("PASS DSP guest-address mapping, shared producer/consumer registry, span checks, lifecycle and concurrent registration");
}
