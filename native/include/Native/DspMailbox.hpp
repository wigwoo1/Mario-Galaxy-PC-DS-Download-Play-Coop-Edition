#pragma once

#include <revolution/types.h>

#include <cstddef>
#include <cstdint>
#include <mutex>

namespace Native {

class DspMailbox {
public:
    using InterruptHandler = void (*)(void* context);

    bool tryWriteToDsp(u32 word) noexcept;
    bool tryReadToDsp(u32* word) noexcept;
    bool tryWriteFromDsp(u32 word) noexcept;
    bool tryReadFromDsp(u32* word) noexcept;
    bool toDspFull() const noexcept;
    bool fromDspFull() const noexcept;
    std::uint64_t protocolErrors() const noexcept;
    void setInterruptHandler(InterruptHandler handler, void* context) noexcept;
    void assertInterrupt() noexcept;
    void resetMailboxes() noexcept;

private:
    mutable std::mutex mutex_;
    u32 toDspWord_ = 0;
    u32 fromDspWord_ = 0;
    bool toDspFull_ = false;
    bool fromDspFull_ = false;
    std::uint64_t protocolErrors_ = 0;
    InterruptHandler interruptHandler_ = nullptr;
    void* interruptContext_ = nullptr;
};

DspMailbox& nativeDspMailbox() noexcept;

} // namespace Native

extern "C" {
// Optional audio renderer owned by the native audio thread. Callbacks/context
// must remain alive until unregistered; registration/teardown is serialized
// with command submission by the owner. Service runs outside the ROM mutex.
using NativeDspAudioProcess=s32(*)(void*,const u32*,u32,u16*);
using NativeDspAudioService=void(*)(void*);
void nativeSetDspHostAudioBackend(NativeDspAudioProcess process,NativeDspAudioService service,void* context);
void nativeDspServiceHostAudio();
using NativeDspRequestCallback=void(*)(void*);
using NativeDspAudioStart=bool(*)(NativeDspRequestCallback);
using NativeDspAudioStop=void(*)();
// Audio thread installs these factories before boot. SDK-only tests may omit
// them; the full audio thread requires a real renderer rather than silence.
void nativeSetDspAudioBootHooks(NativeDspAudioStart start,NativeDspAudioStop stop);
bool nativeDspNotifyAudioReady(NativeDspRequestCallback request);
NativeDspRequestCallback nativeDspBootRequestCallback();
void nativeSetDspMailboxInterrupt(Native::DspMailbox::InterruptHandler handler,
                                  void* context);
// Returns0 before ROM boot,1 for explicit legacy silence,2 when an installed
// renderer owns deferred completion, or a negative error. Status2 alone does
// not mean a frame completed: only its actual completion mail proves that.
s32 nativeDspProcessHostAudioCommand(const u32* words, u32 count,
                                    u16* completionToken);
// Start the tested SMG ROM-upload handshake backend for a uCode image.
// This does not execute Zelda audio commands after startup.
bool nativeDspBeginRomTask(const void* image, std::size_t imageBytes);
}
