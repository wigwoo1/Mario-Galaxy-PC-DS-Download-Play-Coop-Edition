#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include <Native/DspAddressSpace.hpp>
#include <revolution/ai.h>

#include <algorithm>
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
    const std::uint8_t* pendingData = nullptr;
    std::uint32_t pendingBytes = 0;
    std::uint32_t currentAddress = 0;
    std::uint32_t currentBytes = 0;
    std::uint32_t sampleRate = kDefaultSampleRate;
    HWAVEOUT wave = nullptr;
    HANDLE completionEvent = nullptr;
    bool initialized = false;
    bool running = false;
    bool stopping = false;
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
        s.completionEvent = event;
        if (!s.running) {
            s.wave = nullptr;
            s.completionEvent = nullptr;
        }
    }

    std::vector<std::uint8_t> silence;
    WAVEHDR header{};
    for (;;) {
        const std::uint8_t* data = nullptr;
        std::uint32_t bytes = 0;
        AIDCallback callback = nullptr;
        std::uint32_t activeRate = kDefaultSampleRate;
        {
            std::unique_lock<std::mutex> lock(s.mutex);
            s.changed.wait_for(lock, std::chrono::milliseconds(5),
                               [&] { return !s.running || s.pendingData != nullptr; });
            if (!s.running) break;
            data = s.pendingData;
            bytes = s.pendingBytes;
            s.pendingData = nullptr;
            s.pendingBytes = 0;
            callback = s.callback;
            activeRate = s.sampleRate;
        }

        // A completion callback clocks the original audio thread. It may need
        // a few milliseconds to mix and submit its next triple-buffered chunk.
        // If it has not arrived, feed silence at the same cadence instead of
        // spinning or replaying a buffer that the mixer can reuse.
        if (bytes == 0 || bytes > kMaximumDmaBytes) bytes = 2240;
        silence.assign(bytes, 0);
        header = {};
        header.lpData = reinterpret_cast<LPSTR>(const_cast<std::uint8_t*>(data ? data : silence.data()));
        header.dwBufferLength = bytes;

        if (wave != nullptr && event != nullptr) {
            ResetEvent(event);
            MMRESULT result = waveOutPrepareHeader(wave, &header, sizeof(header));
            if (result == MMSYSERR_NOERROR) {
                result = waveOutWrite(wave, &header, sizeof(header));
            }
            if (result == MMSYSERR_NOERROR) {
                const DWORD timeout = chunkDurationMs(bytes, activeRate) + 1000;
                const DWORD waitResult = WaitForSingleObject(event, timeout);
                if (waitResult == WAIT_TIMEOUT) {
                    waveOutReset(wave);
                    reportWaveError("waveOut completion wait", MMSYSERR_ERROR);
                }
                waveOutUnprepareHeader(wave, &header, sizeof(header));
            } else {
                if ((header.dwFlags & WHDR_PREPARED) != 0) {
                    waveOutUnprepareHeader(wave, &header, sizeof(header));
                }
                reportWaveError("waveOutWrite", result);
                waveOutReset(wave);
                waveOutClose(wave);
                wave = nullptr;
                std::lock_guard<std::mutex> lock(s.mutex);
                s.wave = nullptr;
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(chunkDurationMs(bytes, activeRate)));
        }

        if (callback != nullptr) callback();
    }

    if (wave != nullptr) {
        waveOutReset(wave);
        waveOutClose(wave);
    }
    if (event != nullptr) CloseHandle(event);
    {
        std::lock_guard<std::mutex> lock(s.mutex);
        s.wave = nullptr;
        s.completionEvent = nullptr;
        s.pendingData = nullptr;
        s.pendingBytes = 0;
    }
}

} // namespace

extern "C" void AIInit(u8*) {
    AIStopDMA();
    AiDmaState& s = state();
    std::lock_guard<std::mutex> lock(s.mutex);
    s.initialized = true;
    s.sampleRate = kDefaultSampleRate;
    s.pendingData = nullptr;
    s.pendingBytes = 0;
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
        s.pendingData = data;
        s.pendingBytes = data ? length : 0;
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
    if (!s.initialized || s.running || s.stopping) return;
    s.running = true;
    s.worker = std::thread(audioWorker);
}

extern "C" void AIStopDMA(void) {
    AiDmaState& s = state();
    std::thread worker;
    HWAVEOUT wave = nullptr;
    HANDLE event = nullptr;
    {
        std::lock_guard<std::mutex> lock(s.mutex);
        s.running = false;
        wave = s.wave;
        event = s.completionEvent;
        if (s.worker.joinable()) {
            if (s.worker.get_id() == std::this_thread::get_id()) return;
            s.stopping = true;
            worker = std::move(s.worker);
        }
    }
    s.changed.notify_all();
    if (wave != nullptr) waveOutReset(wave);
    if (event != nullptr) SetEvent(event);
    if (worker.joinable()) worker.join();
    {
        std::lock_guard<std::mutex> lock(s.mutex);
        s.stopping = false;
    }
    s.changed.notify_all();
}
