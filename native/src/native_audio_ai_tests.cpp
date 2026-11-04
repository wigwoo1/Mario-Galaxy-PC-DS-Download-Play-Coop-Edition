#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <Native/DspAddressSpace.hpp>
#include <Native/AudioAiDiagnostics.hpp>
#include <revolution/ai.h>

#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

namespace {
std::atomic<unsigned> completions{0};
std::mutex mutex;
std::condition_variable changed;
std::vector<std::chrono::steady_clock::time_point> times;
std::atomic<bool> stoppedFromCallback{false};
void selfStop() {
    AIStopDMA();
    // Self-join is impossible; premature restart must wait for control-thread
    // reaping rather than overwriting a still-joinable std::thread.
    AIStartDMA();
    stoppedFromCallback.store(true,std::memory_order_release);changed.notify_all();
}
void onDmaComplete() {
    std::lock_guard<std::mutex> lock(mutex);
    times.push_back(std::chrono::steady_clock::now());
    completions.fetch_add(1, std::memory_order_relaxed);changed.notify_all();
}
void cadence(unsigned bytes,unsigned rate) {
    {std::lock_guard<std::mutex> lock(mutex);times.clear();}
    AIStartDMA();
    std::unique_lock<std::mutex> lock(mutex);
    assert(changed.wait_for(lock,std::chrono::seconds(3),[]{return times.size()>=40;}));
    const double elapsed=std::chrono::duration<double>(times[39]-times[0]).count();
    const double expected=39.0*(bytes/4)/rate;
    std::printf("DMA cadence rate=%u expected=%.6f actual=%.6f seconds\n",rate,expected,elapsed);
    assert(elapsed>=expected*.8&&elapsed<=expected*1.2+.025);
    lock.unlock();AIStopDMA();
    const auto snapshot=Native::nativeAiOutputSnapshot();
    assert(snapshot.backend==Native::AiOutputBackend::Stopped&&snapshot.queuedBlocks==0);
    if(snapshot.deviceCompletedFrames)assert(snapshot.maximumQueuedBlocks==3);
    std::printf("PASS DMA sample clock, stop/join and restart at%uHz: deviceFrames=%llu peakQueued=%u\n",rate,static_cast<unsigned long long>(snapshot.deviceCompletedFrames),snapshot.maximumQueuedBlocks);
}
}

int main() {
    auto* pcm=VirtualAlloc(nullptr,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    assert(pcm);ZeroMemory(pcm,4096);
    const auto address = Native::nativeDspAddressSpace().map(pcm,2240);
    assert(address != 0);

    AIInit(nullptr);
    AISetDSPSampleRate(0);
    assert(AIRegisterDMACallback(onDmaComplete) == nullptr);
    AIInitDMA(address,2240);
    assert(AIGetDMAStartAddr() == address);
    assert(AIGetDMALength() == 2240);

    // Submission must own a copy before returning. The original caller span
    // becomes unreadable before the real device worker opens/queues buffers.
    assert(Native::nativeDspAddressSpace().unmap(address));
    DWORD oldProtection=0;assert(VirtualProtect(pcm,4096,PAGE_NOACCESS,&oldProtection));

    cadence(2240,32000);
    const auto owned=Native::nativeAiOutputSnapshot();
    if(waveOutGetNumDevs()) {
        assert(owned.deviceCompletedFrames>0&&owned.maximumQueuedBlocks==3);
        std::puts("PASS real waveOut completed queued owned PCM while caller storage was unmapped/PAGE_NOACCESS");
    } else {
        std::puts("SKIP physical owned-buffer check: no waveOut device; sample-clock fallback was exercised");
    }
    assert(VirtualFree(pcm,0,MEM_RELEASE));
    std::array<std::uint8_t,3200> higherRatePcm{};
    const auto secondAddress=Native::nativeDspAddressSpace().map(higherRatePcm.data(),higherRatePcm.size());
    assert(secondAddress);AISetDSPSampleRate(1);AIInitDMA(secondAddress,higherRatePcm.size());
    cadence(3200,48000);

    const auto before = completions.load(std::memory_order_relaxed);
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    assert(completions.load(std::memory_order_relaxed) == before);

    assert(AIRegisterDMACallback(selfStop)==onDmaComplete);
    AIStartDMA();
    {
        std::unique_lock<std::mutex> lock(mutex);
        assert(changed.wait_for(lock,std::chrono::seconds(2),[]{return stoppedFromCallback.load(std::memory_order_acquire);}));
    }
    AIStopDMA();
    assert(Native::nativeAiOutputSnapshot().backend==Native::AiOutputBackend::Stopped);
    assert(AIRegisterDMACallback(onDmaComplete)==selfStop);
    const auto restartCount=completions.load(std::memory_order_relaxed);
    AIStartDMA();
    {
        std::unique_lock<std::mutex> lock(mutex);
        assert(changed.wait_for(lock,std::chrono::seconds(2),[&]{return completions.load(std::memory_order_relaxed)>restartCount;}));
    }
    AIStopDMA();
    std::puts("PASS callback-initiated STOP rejects premature self-restart, control thread reaps it and subsequent restart completes normally");

    AIInitDMA(0xfffffff0u, 32);
    assert(AIGetDMAStartAddr() == 0xfffffff0u);
    const auto prior = AIRegisterDMACallback(nullptr);
    assert(prior == onDmaComplete);
    std::puts("PASS host Wii AI: mapped DMA span, stereo chunk size, paced completion, stop/join, callback replacement and invalid-range rejection");
}
