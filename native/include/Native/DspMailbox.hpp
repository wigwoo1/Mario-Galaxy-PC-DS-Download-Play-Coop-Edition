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
void nativeSetDspMailboxInterrupt(Native::DspMailbox::InterruptHandler handler,
                                  void* context);
// Returns 0 until the verified SMG ROM boot HLE is running, 1 when a command
// batch was accepted by the silent host control backend, or a negative status
// for a malformed/unsupported packet. Accepted packets do not produce PCM.
s32 nativeDspProcessHostAudioCommand(const u32* words, u32 count,
                                    u16* completionToken);
// Start the tested SMG ROM-upload handshake backend for a uCode image.
// This does not execute Zelda audio commands after startup.
bool nativeDspBeginRomTask(const void* image, std::size_t imageBytes);
}
