#include <Native/DspAddressSpace.hpp>
#include <revolution/ai.h>

#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <thread>

namespace {
std::atomic<unsigned> completions{0};
void onDmaComplete() { completions.fetch_add(1, std::memory_order_relaxed); }
}

int main() {
    std::array<std::uint8_t, 2240> pcm{};
    const auto address = Native::nativeDspAddressSpace().map(pcm.data(), pcm.size());
    assert(address != 0);

    AIInit(nullptr);
    AISetDSPSampleRate(0);
    assert(AIRegisterDMACallback(onDmaComplete) == nullptr);
    AIInitDMA(address, static_cast<u32>(pcm.size()));
    assert(AIGetDMAStartAddr() == address);
    assert(AIGetDMALength() == pcm.size());

    AIStartDMA();
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (completions.load(std::memory_order_relaxed) < 3 &&
           std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    AIStopDMA();
    assert(completions.load(std::memory_order_relaxed) >= 3);

    const auto before = completions.load(std::memory_order_relaxed);
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    assert(completions.load(std::memory_order_relaxed) == before);

    AIInitDMA(0xfffffff0u, 32);
    assert(AIGetDMAStartAddr() == 0xfffffff0u);
    const auto prior = AIRegisterDMACallback(nullptr);
    assert(prior == onDmaComplete);
    std::puts("PASS host Wii AI: mapped DMA span, stereo chunk size, paced completion, stop/join, callback replacement and invalid-range rejection");
}
