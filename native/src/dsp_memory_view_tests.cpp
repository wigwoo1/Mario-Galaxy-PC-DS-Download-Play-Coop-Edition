#include <Native/DspMemoryView.hpp>

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>

int main() {
    Native::DspAddressSpace addressSpace;
    Native::DspMemoryView memory(addressSpace);
    alignas(32) std::array<std::uint8_t, 32> bytes{};
    const auto base = addressSpace.map(bytes.data(), bytes.size());
    assert(base != 0);

    bytes[0] = 0x12; bytes[1] = 0x34; bytes[2] = 0x56; bytes[3] = 0x78;
    std::uint16_t word16 = 0;
    std::uint32_t word32 = 0;
    assert(memory.readBigEndian16(base, word16) && word16 == 0x1234);
    assert(memory.readBigEndian32(base, word32) && word32 == 0x12345678);

    assert(memory.writeBigEndian16(base + 4, 0xabcd));
    assert(bytes[4] == 0xab && bytes[5] == 0xcd);
    assert(memory.writeBigEndian32(base + 8, 0x89abcdef));
    assert(bytes[8] == 0x89 && bytes[9] == 0xab && bytes[10] == 0xcd && bytes[11] == 0xef);

    const std::uint32_t hostValue = 0x12345678;
    assert(memory.writeHost(base + 12, hostValue));
    std::uint32_t hostRead = 0;
    assert(memory.readHost(base + 12, hostRead) && hostRead == hostValue);
    assert(bytes[12] == reinterpret_cast<const std::uint8_t*>(&hostValue)[0]);

    const std::array<std::uint8_t, 4> bigEndianSamples{0x80, 0x01, 0x12, 0x34};
    assert(memory.writeBytes(base + 16, bigEndianSamples.data(), bigEndianSamples.size()));
    std::array<std::uint16_t, 2> hostSamples{};
    assert(memory.copyBigEndianWordsToHost(base + 16, hostSamples.data(), sizeof(hostSamples)));
    assert(hostSamples[0] == 0x8001 && hostSamples[1] == 0x1234);

    assert(!memory.readBigEndian32(base + 30, word32));
    assert(!memory.writeBigEndian16(base + 31, 0x1234));
    assert(!memory.readBytes(base, nullptr, 1));
    assert(!memory.writeBytes(base, nullptr, 1));
    assert(!memory.copyBigEndianWordsToHost(base, hostSamples.data(), 3));
    assert(memory.readBytes(base + bytes.size(), nullptr, 0));
    assert(!memory.readBytes(0, nullptr, 0));

    std::puts("PASS DSP bounded memory view, host-native access, guest endian conversion and range rejection");
}
