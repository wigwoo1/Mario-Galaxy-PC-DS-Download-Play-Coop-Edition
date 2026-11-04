#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include <Native/DspAddressSpace.hpp>
#include <Native/AudioAiDiagnostics.hpp>
#include <revolution/ai.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>

namespace {

constexpr std::uint32_t kDefaultSampleRate = 32000;
constexpr std::uint32_t kMaximumDmaBytes = 1024u * 1024u;

struct AiDmaState {
    std::mutex mutex;
    std::condition_variable changed;
    std::thread worker;
    AIDCallback callback = nullptr;
    std::vector<std::uint8_t> pendingPcm;
    std::uint32_t currentAddress = 0;
    std::uint32_t currentBytes = 0;
    std::uint32_t sampleRate = kDefaultSampleRate;
    HWAVEOUT wave = nullptr;
    HANDLE completionEvent = nullptr;
    bool initialized = false;
    bool running = false;
    bool stopping = false;
    Native::AiOutputBackend backend = Native::AiOutputBackend::Stopped;
    std::uint32_t queuedBlocks = 0;
    std::uint32_t maximumQueuedBlocks = 0;
    std::uint64_t completedFrames = 0;
    std::uint64_t deviceCompletedFrames = 0;
};

AiDmaState& state() {
    // Intentionally process-lifetime: the game explicitly calls AIStopDMA
    // during audio shutdown, and late OS audio callbacks must not race a
    // static C++ destructor.
    static AiDmaState* instance = new AiDmaState;
    return *instance;
}

std::uint32_t chunkDurationMs(std::uint32_t bytes, std::uint32_t rate) {
    const std::uint32_t frames = std::max<std::uint32_t>(bytes / 4, 1);
    const std::uint32_t safeRate = std::max<std::uint32_t>(rate, 1);
    return std::max<std::uint32_t>(1, (frames * 1000u + safeRate - 1) / safeRate);
}

void reportWaveError(const char* operation, MMRESULT result) {
    char message[160]{};
    wsprintfA(message, "Galaxy native audio: %s failed (MMRESULT=%u); using silent DMA clock.\n",
              operation, static_cast<unsigned>(result));
    OutputDebugStringA(message);
}

void audioWorker() {
    AiDmaState& s = state();
    HANDLE event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    HWAVEOUT wave = nullptr;
    std::uint32_t sampleRate = kDefaultSampleRate;
    {
        std::lock_guard<std::mutex> lock(s.mutex);
        sampleRate = s.sampleRate;
    }

    WAVEFORMATEX format{};
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 2;
    format.nSamplesPerSec = sampleRate;
    format.wBitsPerSample = 16;
    format.nBlockAlign = 4;
    format.nAvgBytesPerSec = sampleRate * format.nBlockAlign;
    format.cbSize = 0;

    MMRESULT openResult = event
        ? waveOutOpen(&wave, WAVE_MAPPER, &format,
                      reinterpret_cast<DWORD_PTR>(event), 0, CALLBACK_EVENT)
        : MMSYSERR_NOMEM;
    if (openResult != MMSYSERR_NOERROR) {
        reportWaveError("waveOutOpen", openResult);
        wave = nullptr;
    }

    {
        std::lock_guard<std::mutex> lock(s.mutex);
        s.wave = wave;
        s.backend = wave ? Native::AiOutputBackend::WaveOut : Native::AiOutputBackend::SilentClock;
        s.completionEvent = event;
        if (!s.running) {
            s.wave = nullptr;
            s.completionEvent = nullptr;
        }
    }

    auto running = [&] {
        std::lock_guard<std::mutex> lock(s.mutex);
        return s.running;
    };
    auto nextPcm = [&](bool wait) {
        std::unique_lock<std::mutex> lock(s.mutex);
        if (wait) s.changed.wait_for(lock, std::chrono::milliseconds(5),
                                    [&] { return !s.running || !s.pendingPcm.empty(); });
        auto pcm = std::move(s.pendingPcm);
        s.pendingPcm.clear();
        if (pcm.empty()) {
            const auto bytes = s.currentBytes >= 4 && s.currentBytes <= kMaximumDmaBytes &&
                               s.currentBytes % 4 == 0 ? s.currentBytes : 2240;
            pcm.assign(bytes, 0);
        }
        return pcm;
    };
    auto complete = [&](std::size_t bytes, bool device) {
        AIDCallback callback = nullptr;
        {
            std::lock_guard<std::mutex> lock(s.mutex);
            if (s.running) {
                s.completedFrames += bytes / 4;
                if (device) {
                    s.deviceCompletedFrames += bytes / 4;
                    --s.queuedBlocks;
                }
                callback = s.callback;
            }
        }
        if (callback) callback();
    };
    struct Chunk {
        WAVEHDR header{};
        std::vector<std::uint8_t> pcm;
    };
    // Keep data queued ahead of the driver. Waiting for one buffer and only
    // then writing its successor repeatedly incurred endpoint startup gaps.
    // Copies own their PCM until WHDR_DONE; the game may rotate its DMA arrays.
    std::array<Chunk, 3> chunks;
    auto submit = [&](Chunk& chunk, bool wait) {
        chunk.pcm = nextPcm(wait);
        if (!running()) return false;
        chunk.header = {};
        chunk.header.lpData = reinterpret_cast<LPSTR>(chunk.pcm.data());
        chunk.header.dwBufferLength = static_cast<DWORD>(chunk.pcm.size());
        MMRESULT result = waveOutPrepareHeader(wave, &chunk.header, sizeof(WAVEHDR));
        if (result == MMSYSERR_NOERROR) result = waveOutWrite(wave, &chunk.header, sizeof(WAVEHDR));
        if (result != MMSYSERR_NOERROR) reportWaveError("waveOut queue", result);
        else {
            std::lock_guard<std::mutex> lock(s.mutex);
            ++s.queuedBlocks;
            s.maximumQueuedBlocks = std::max(s.maximumQueuedBlocks, s.queuedBlocks);
        }
        return result == MMSYSERR_NOERROR;
    };
    auto closeWave = [&] {
        if (!wave) return;
        // Only this worker operates/closes the device; STOP wakes its event.
        waveOutReset(wave);
        for (auto& chunk : chunks) {
            if (chunk.header.dwFlags & WHDR_PREPARED)
                waveOutUnprepareHeader(wave, &chunk.header, sizeof(WAVEHDR));
        }
        waveOutClose(wave);
        wave = nullptr;
        std::lock_guard<std::mutex> lock(s.mutex);
        s.wave = nullptr;
        s.queuedBlocks = 0;
        s.backend = s.running ? Native::AiOutputBackend::SilentClock : Native::AiOutputBackend::Stopped;
    };
    if (wave) {
        bool queued = true;
        for (auto& chunk : chunks) if (!submit(chunk, false)) { queued = false; break; }
        unsigned oldest = 0;
        while (queued && running()) {
            auto& chunk = chunks[oldest];
            if (!(chunk.header.dwFlags & WHDR_DONE)) {
                const DWORD timeout = chunkDurationMs(static_cast<u32>(chunk.pcm.size()), sampleRate) + 1000;
                if (WaitForSingleObject(event, timeout) == WAIT_TIMEOUT) {
                    reportWaveError("waveOut completion wait", MMSYSERR_ERROR);
                    break;
                }
                // Open/close/state-change events are not DMA completion.
                continue;
            }
            const auto result = waveOutUnprepareHeader(wave, &chunk.header, sizeof(WAVEHDR));
            if (result != MMSYSERR_NOERROR) { reportWaveError("waveOut unprepare", result); break; }
            complete(chunk.pcm.size(), true);
            queued = submit(chunk, true);
            oldest = (oldest + 1) % chunks.size();
        }
        closeWave();
    }
    // If no endpoint is available, retain the original DMA clock without
    // adding a rounded sleep and mixer wait to every block's duration.
    auto deadline = std::chrono::steady_clock::now();
    while (running()) {
        const auto pcm = nextPcm(true);
        const auto nanos = std::uint64_t(pcm.size() / 4) * 1000000000ull / sampleRate;
        deadline += std::chrono::nanoseconds(nanos);
        {
            std::unique_lock<std::mutex> lock(s.mutex);
            s.changed.wait_until(lock, deadline, [&] { return !s.running; });
            if (!s.running) break;
        }
        complete(pcm.size(), false);
    }
    {
        std::lock_guard<std::mutex> lock(s.mutex);
        s.wave = nullptr;
        s.backend = Native::AiOutputBackend::Stopped;
        s.completionEvent = nullptr;
        s.pendingPcm.clear();
    }
    if (event) CloseHandle(event);
}

} // namespace

Native::AiOutputSnapshot Native::nativeAiOutputSnapshot() noexcept {
    auto& s = state();
    std::lock_guard<std::mutex> lock(s.mutex);
    return {s.backend, s.sampleRate, s.queuedBlocks, s.maximumQueuedBlocks,
            s.completedFrames, s.deviceCompletedFrames};
}

extern "C" void AIInit(u8*) {
    AIStopDMA();
    AiDmaState& s = state();
    std::lock_guard<std::mutex> lock(s.mutex);
    s.initialized = true;
    s.sampleRate = kDefaultSampleRate;
    s.pendingPcm.clear();
    s.currentAddress = 0;
    s.currentBytes = 0;
}

extern "C" void AIInitDMA(u32 address, u32 length) {
    AiDmaState& s = state();
    const bool validLength = length >= 4 && length <= kMaximumDmaBytes && (length % 4) == 0;
    auto* data = validLength
        ? static_cast<const std::uint8_t*>(Native::nativeDspAddressSpace().resolve(address, length))
        : nullptr;
    {
        std::lock_guard<std::mutex> lock(s.mutex);
        s.currentAddress = address;
        s.currentBytes = length;
        if (data) s.pendingPcm.assign(data, data + length);
        else s.pendingPcm.clear();
    }
    if (data == nullptr) {
        OutputDebugStringA("Galaxy native audio: rejected invalid Wii AI DMA guest range.\n");
    }
    s.changed.notify_one();
}

extern "C" AIDCallback AIRegisterDMACallback(AIDCallback callback) {
    AiDmaState& s = state();
    std::lock_guard<std::mutex> lock(s.mutex);
    AIDCallback previous = s.callback;
    s.callback = callback;
    return previous;
}

extern "C" void AISetDSPSampleRate(u32 rateMode) {
    AiDmaState& s = state();
    bool restart = false;
    {
        std::lock_guard<std::mutex> lock(s.mutex);
        restart = s.running;
    }
    if (restart) AIStopDMA();
    {
        std::lock_guard<std::mutex> lock(s.mutex);
        s.sampleRate = rateMode == 0 ? 32000u : 48000u;
    }
    if (restart) AIStartDMA();
}

extern "C" u32 AIGetDMAStartAddr(void) {
    AiDmaState& s = state();
    std::lock_guard<std::mutex> lock(s.mutex);
    return s.currentAddress;
}

extern "C" u32 AIGetDMALength(void) {
    AiDmaState& s = state();
    std::lock_guard<std::mutex> lock(s.mutex);
    return s.currentBytes;
}

extern "C" void AIStartDMA(void) {
    AiDmaState& s = state();
    std::lock_guard<std::mutex> lock(s.mutex);
    // A callback can request STOP but cannot join itself. Its control thread
    // must reap that worker with AIStopDMA before starting another one.
    if (!s.initialized || s.running || s.stopping || s.worker.joinable()) return;
    s.queuedBlocks = s.maximumQueuedBlocks = 0;
    s.completedFrames = s.deviceCompletedFrames = 0;
    s.running = true;
    s.worker = std::thread(audioWorker);
}

extern "C" void AIStopDMA(void) {
    AiDmaState& s = state();
    std::thread worker;
    {
        std::lock_guard<std::mutex> lock(s.mutex);
        s.running = false;
        if (s.completionEvent) SetEvent(s.completionEvent);
        if (s.worker.joinable()) {
            if (s.worker.get_id() == std::this_thread::get_id()) return;
            s.stopping = true;
            worker = std::move(s.worker);
        }
    }
    s.changed.notify_all();
    if (worker.joinable()) worker.join();
    {
        std::lock_guard<std::mutex> lock(s.mutex);
        s.stopping = false;
    }
    s.changed.notify_all();
}
