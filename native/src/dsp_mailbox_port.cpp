#include <Native/DspMailbox.hpp>
#include <Native/DspAddressSpace.hpp>
#include <Native/DspAudioCommandHle.hpp>
#include <Native/DspRomBootHle.hpp>

#include <memory>
#include <mutex>

namespace Native {
namespace {
std::mutex gRomRuntimeMutex;
std::unique_ptr<DspRomBootHle> gRomBoot;
DspAddressSpace::Address gRomImageMapping = 0;
bool gRomImageMappingOwned = false;
DspAudioCommandHle gAudioCommandHle;

void serviceRomBootLocked(DspMailbox& mailbox) noexcept {
    if (!gRomBoot) return;
    for (;;) {
        if (!mailbox.fromDspFull()) {
            u32 reply = 0;
            if (gRomBoot->tryReadDspMail(&reply)) {
                if (!mailbox.tryWriteFromDsp(reply)) return;
                continue;
            }
        }
        if (mailbox.fromDspFull()) return;

        u32 command = 0;
        if (!mailbox.tryReadToDsp(&command)) return;
        if (!gRomBoot->acceptCpuMail(command)) return;
    }
}

void resetRomBootLocked(DspMailbox& mailbox) noexcept {
    gRomBoot.reset();
    if (gRomImageMapping && gRomImageMappingOwned) {
        nativeDspAddressSpace().unmap(gRomImageMapping);
    }
    gRomImageMapping = 0;
    gRomImageMappingOwned = false;
    mailbox.resetMailboxes();
}
} // namespace

DspMailbox& nativeDspMailbox() noexcept {
    static DspMailbox mailbox;
    return mailbox;
}

bool DspMailbox::tryWriteToDsp(u32 word) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    if (toDspFull_) {
        ++protocolErrors_;
        return false;
    }
    toDspWord_ = word;
    toDspFull_ = true;
    return true;
}

bool DspMailbox::tryReadToDsp(u32* word) noexcept {
    if (!word) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    if (!toDspFull_) return false;
    *word = toDspWord_;
    toDspFull_ = false;
    toDspWord_ = 0;
    return true;
}

bool DspMailbox::tryWriteFromDsp(u32 word) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    if (fromDspFull_) {
        ++protocolErrors_;
        return false;
    }
    fromDspWord_ = word;
    fromDspFull_ = true;
    return true;
}

bool DspMailbox::tryReadFromDsp(u32* word) noexcept {
    if (!word) return false;
    std::lock_guard<std::mutex> lock(mutex_);
    if (!fromDspFull_) return false;
    *word = fromDspWord_;
    fromDspFull_ = false;
    fromDspWord_ = 0;
    return true;
}

bool DspMailbox::toDspFull() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return toDspFull_;
}

bool DspMailbox::fromDspFull() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return fromDspFull_;
}

std::uint64_t DspMailbox::protocolErrors() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return protocolErrors_;
}

void DspMailbox::setInterruptHandler(InterruptHandler handler,
                                     void* context) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    interruptHandler_ = handler;
    interruptContext_ = context;
}

void DspMailbox::assertInterrupt() noexcept {
    InterruptHandler handler = nullptr;
    void* context = nullptr;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        handler = interruptHandler_;
        context = interruptContext_;
    }
    if (handler) handler(context);
}

void DspMailbox::resetMailboxes() noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    toDspWord_ = 0;
    fromDspWord_ = 0;
    toDspFull_ = false;
    fromDspFull_ = false;
    protocolErrors_ = 0;
}

} // namespace Native

extern "C" void nativeSetDspMailboxInterrupt(
    Native::DspMailbox::InterruptHandler handler, void* context) {
    Native::nativeDspMailbox().setInterruptHandler(handler, context);
}

extern "C" s32 nativeDspProcessHostAudioCommand(const u32* words, u32 count,
                                                 u16* completionToken) {
    std::lock_guard<std::mutex> lock(Native::gRomRuntimeMutex);
    if (!Native::gRomBoot ||
        Native::gRomBoot->state() != Native::DspRomBootHle::State::Running)
        return static_cast<s32>(Native::DspAudioCommandHle::Result::NoBackend);

    return static_cast<s32>(
        Native::gAudioCommandHle.processBatch(words, count, completionToken));
}

extern "C" u32 DSPCheckMailToDSP(void) {
    auto& mailbox = Native::nativeDspMailbox();
    {
        std::lock_guard<std::mutex> lock(Native::gRomRuntimeMutex);
        Native::serviceRomBootLocked(mailbox);
    }
    return mailbox.toDspFull() ? 1u : 0u;
}

extern "C" u32 DSPCheckMailFromDSP(void) {
    auto& mailbox = Native::nativeDspMailbox();
    {
        std::lock_guard<std::mutex> lock(Native::gRomRuntimeMutex);
        Native::serviceRomBootLocked(mailbox);
    }
    return mailbox.fromDspFull() ? 1u : 0u;
}

extern "C" u32 DSPReadMailFromDSP(void) {
    u32 word = 0;
    auto& mailbox = Native::nativeDspMailbox();
    std::lock_guard<std::mutex> lock(Native::gRomRuntimeMutex);
    mailbox.tryReadFromDsp(&word);
    Native::serviceRomBootLocked(mailbox);
    return word;
}

extern "C" void DSPSendMailToDSP(u32 word) {
    auto& mailbox = Native::nativeDspMailbox();
    std::lock_guard<std::mutex> lock(Native::gRomRuntimeMutex);
    mailbox.tryWriteToDsp(word);
    Native::serviceRomBootLocked(mailbox);
}

extern "C" void DSPAssertInt(void) {
    Native::nativeDspMailbox().assertInterrupt();
}

extern "C" void DSPInit(void) {
    std::lock_guard<std::mutex> lock(Native::gRomRuntimeMutex);
    Native::resetRomBootLocked(Native::nativeDspMailbox());
}

extern "C" bool nativeDspBeginRomTask(const void* image,
                                         std::size_t imageBytes) {
    auto& mailbox = Native::nativeDspMailbox();
    std::lock_guard<std::mutex> lock(Native::gRomRuntimeMutex);
    Native::resetRomBootLocked(mailbox);
    auto mapping = Native::nativeDspAddressSpace().addressOf(image, imageBytes);
    const bool ownsMapping = mapping == 0;
    if (ownsMapping)
        mapping = Native::nativeDspAddressSpace().map(image, imageBytes);
    if (!mapping) return false;
    auto boot = std::make_unique<Native::DspRomBootHle>(
        Native::nativeDspAddressSpace());
    if (!boot->begin(mapping, imageBytes)) {
        if (ownsMapping) Native::nativeDspAddressSpace().unmap(mapping);
        return false;
    }
    Native::gRomImageMapping = mapping;
    Native::gRomImageMappingOwned = ownsMapping;
    Native::gRomBoot = std::move(boot);
    Native::serviceRomBootLocked(mailbox);
    return mailbox.fromDspFull();
}
