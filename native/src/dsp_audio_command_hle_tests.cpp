#include <Native/DspAudioCommandHle.hpp>

#include <cassert>
#include <cstdint>
#include <cstdio>

int main() {
    Native::DspAudioCommandHle hle;
    std::uint16_t token = 0;

    const std::uint32_t varam[] = {0x8E000000u, 0x12345678u};
    assert(hle.processBatch(varam, 2, &token) ==
           Native::DspAudioCommandHle::Result::AcceptedSilent);
    assert(token == 0x8E00u);

    const std::uint32_t table[] = {
        0x81000040u, 0x1000u, 0x2000u, 0x3000u, 0x4000u
    };
    assert(hle.processBatch(table, 5, &token) ==
           Native::DspAudioCommandHle::Result::AcceptedSilent);
    assert(token == 0x8100u);

    const std::uint32_t frame[] = {
        0x82072000u, 0x12345678u, 0x87654321u, 0u, 0u
    };
    assert(hle.processBatch(frame, 5, &token) ==
           Native::DspAudioCommandHle::Result::AcceptedSilent);
    assert(token == 0x8207u);

    const std::uint32_t halt[] = {0x00551234u};
    assert(hle.processBatch(halt, 0, &token) ==
           Native::DspAudioCommandHle::Result::AcceptedSilent);
    assert(token == 0x0055u);

    assert(hle.processBatch(varam, 1, &token) ==
           Native::DspAudioCommandHle::Result::Malformed);
    const std::uint32_t badFrame[] = {0x82072000u, 1u};
    assert(hle.processBatch(badFrame, 2, &token) ==
           Native::DspAudioCommandHle::Result::Malformed);
    const std::uint32_t unsupported[] = {0x8D000000u};
    assert(hle.processBatch(unsupported, 1, &token) ==
           Native::DspAudioCommandHle::Result::Unsupported);
    assert(hle.processBatch(nullptr, 1, &token) ==
           Native::DspAudioCommandHle::Result::Malformed);
    assert(hle.processBatch(varam, 2, nullptr) ==
           Native::DspAudioCommandHle::Result::Malformed);

    std::puts("PASS bounded host DSP command validation and explicit silent completion tokens");
}
