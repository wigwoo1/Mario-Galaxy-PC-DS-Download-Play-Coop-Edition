#include "native_dvd_port.hpp"
#include "native_jkr_aram_port.hpp"
#include "native_disc_archive_loader.hpp"
#include "native_disc_font_backend.hpp"
#include "native_disc_message_backend.hpp"
#include <Native/HeapArena.hpp>
#include <Native/GameMessages.hpp>
#include <Native/InputWorkspace.hpp>
#include <Native/VideoBinding.hpp>
#include <Native/VideoRetraceScheduler.hpp>

#include <chrono>
#include <cstring>
#include <exception>
#include <string>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

extern "C" void nativeOriginalGameMain(void);

namespace {
thread_local std::string gameStartError;
class HostMemoryArenas {
public:
    HostMemoryArenas() {
        constexpr std::size_t mem1Bytes = 64u * 1024u * 1024u;
        constexpr std::size_t mem2Bytes = 128u * 1024u * 1024u;
        mem1 = VirtualAlloc(nullptr, mem1Bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        mem2 = VirtualAlloc(nullptr, mem2Bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (!mem1 || !mem2) {
            if (mem1) VirtualFree(mem1, 0, MEM_RELEASE);
            if (mem2) VirtualFree(mem2, 0, MEM_RELEASE);
            mem1 = mem2 = nullptr;
            throw std::runtime_error("Could not reserve host MEM1/MEM2 game heaps");
        }
        if (!nativeBindHeapArena(mem1, mem1Bytes) ||
            !nativeBindSecondaryHeapArena(mem2, mem2Bytes)) {
            nativeReleaseHeapArena();
            VirtualFree(mem1, 0, MEM_RELEASE);
            VirtualFree(mem2, 0, MEM_RELEASE);
            mem1 = mem2 = nullptr;
            throw std::runtime_error("Could not bind host MEM1/MEM2 game heaps");
        }
    }
    ~HostMemoryArenas() {
        const bool secondaryReleased = nativeReleaseSecondaryHeapArena();
        const bool primaryReleased = nativeReleaseHeapArena();
        if (secondaryReleased && primaryReleased) {
            if (mem1) VirtualFree(mem1, 0, MEM_RELEASE);
            if (mem2) VirtualFree(mem2, 0, MEM_RELEASE);
        }
    }
    HostMemoryArenas(const HostMemoryArenas&) = delete;
    HostMemoryArenas& operator=(const HostMemoryArenas&) = delete;
private:
    void* mem1 = nullptr;
    void* mem2 = nullptr;
};
class HostAramBacking {
public:
    HostAramBacking() {
        constexpr std::size_t bytes = 16u * 1024u * 1024u;
        memory = VirtualAlloc(nullptr, bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (!memory) throw std::runtime_error("Could not reserve host ARAM backing");
        transport = std::make_shared<NativeAramMemory>(memory, bytes);
        if (!nativeBindAramTransport(transport)) {
            transport.reset();
            VirtualFree(memory, 0, MEM_RELEASE);
            memory = nullptr;
            throw std::runtime_error("Could not bind host ARAM backing");
        }
    }
    ~HostAramBacking() {
        nativeReleaseAramTransport();
        transport.reset();
        if (memory) VirtualFree(memory, 0, MEM_RELEASE);
    }
    HostAramBacking(const HostAramBacking&) = delete;
    HostAramBacking& operator=(const HostAramBacking&) = delete;
private:
    void* memory = nullptr;
    std::shared_ptr<NativeAramMemory> transport;
};
void appendBootTrace(const char* step) noexcept {
    struct TraceFile {
        SRWLOCK lock = SRWLOCK_INIT;
        HANDLE handle = INVALID_HANDLE_VALUE;
        ~TraceFile() { if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle); }
    };
    static TraceFile trace;
    AcquireSRWLockExclusive(&trace.lock);
    if (trace.handle == INVALID_HANDLE_VALUE) {
        wchar_t path[MAX_PATH]{};
        const DWORD length = GetTempPathW(MAX_PATH, path);
        if (length && length < MAX_PATH - 24) {
            wcscat_s(path, L"galaxy-native-boot.log");
            trace.handle = CreateFileW(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                       nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        }
    }
    if (trace.handle != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(trace.handle, step, static_cast<DWORD>(strlen(step)), &written, nullptr);
        WriteFile(trace.handle, "\r\n", 2, &written, nullptr);
    }
    ReleaseSRWLockExclusive(&trace.lock);
}
}

extern "C" __declspec(dllexport) void galaxyNativeBootTrace(const char* step) noexcept {
    appendBootTrace(step ? step : "<null>");
}

extern "C" __declspec(dllexport) const char* galaxyNativeGameError(void) noexcept {
    return gameStartError.c_str();
}

// Bind the user-supplied disc image before entering Petari's original boot
// sequence. The game entrypoint owns the main loop and normally never returns.
extern "C" __declspec(dllexport) int galaxyNativeStartGame(const wchar_t* imagePath) noexcept {
    gameStartError.clear();
    try {
        appendBootTrace("launcher: start export entered");
        if (!imagePath || !*imagePath) {
            gameStartError = "A Super Mario Galaxy WBFS path is required.";
            return 1;
        }
        if (!nativeBindDvd(std::filesystem::path(imagePath))) {
            gameStartError = "Could not mount the WBFS image. Check that it is a supported, readable single-disc image.";
            return 2;
        }
        appendBootTrace("launcher: WBFS mounted");
        HostMemoryArenas arenas;
        appendBootTrace("launcher: MEM1/MEM2 arenas configured");
        if (!nativeConfigureInputWorkspace(65536)) {
            gameStartError = "Could not configure the native controller workspace.";
            return 5;
        }
        appendBootTrace("launcher: controller workspace configured");
        HostAramBacking aram;
        appendBootTrace("launcher: ARAM backing configured");
        NativeDiscArchiveLoader fontDisc{std::filesystem::path(imagePath)};
        NativeDiscFontBackend fontBackend(fontDisc);
        nativeBindGameFontBackend(&fontBackend);
        appendBootTrace("launcher: retail font backend bound");
        NativeDiscMessageBackend messageBackend(fontDisc);
        nativeBindGameMessageBackend(&messageBackend);
        appendBootTrace("launcher: retail message backend bound");
        Native::VideoInterface video;
        Native::VideoBinding videoBinding(video);
        Native::VideoRetraceScheduler retrace(video, std::chrono::milliseconds(16));
        appendBootTrace("launcher: VI binding and scheduler active");
        nativeOriginalGameMain();
        gameStartError = "The game entrypoint returned unexpectedly.";
        return 3;
    } catch (const std::exception& error) {
        gameStartError = error.what();
    } catch (...) {
        gameStartError = "Unknown failure while mounting the game image.";
    }
    return 4;
}
