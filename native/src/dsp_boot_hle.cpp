#include <Native/DspAddressSpace.hpp>
#include <Native/DspMailbox.hpp>
#include <Native/DspUcodeImage.hpp>

#include <revolution/dsp.h>

void DspInitWork();
void Dsp_Running_Start();

namespace {
constexpr u32 kRomReady = 0x8071FEEDu;
constexpr u32 kDspInit = 0xDCD10000u;
constexpr u32 kZeldaHandshake = 0xF3551111u;

bool receive(u32 expected) {
    return DSPCheckMailFromDSP() != 0 && DSPReadMailFromDSP() == expected;
}

bool send(u32 word) {
    DSPSendMailToDSP(word);
    return DSPCheckMailToDSP() == 0;
}
}

// Host equivalent of Petari's DSPAddPriorTask boot path. It only completes
// the validated ROM/uCode startup exchange; the Zelda audio command processor
// and per-task completion callbacks are not implemented here.
void DspBoot(void (*requestCallback)(void*)) {
    (void)requestCallback;
    DspInitWork();
    DSPInit();

    const auto* image = Native::originalDspUcodeImage();
    const auto imageBytes = Native::originalDspUcodeImageBytes();
    if (!nativeDspBeginRomTask(image, imageBytes) || !receive(kRomReady))
        return;

    const auto guestAddress = Native::nativeDspAddressSpace().addressOf(
        image, imageBytes);
    if (!guestAddress) return;

    constexpr u32 upload[] = {
        0x80F3A001u, 0, 0x80F3C002u, 0,
        0x80F3A002u, 0x1F00u, 0x80F3B002u, 0,
        0x80F3D001u, 0,
    };
    for (std::size_t i = 0; i < sizeof(upload) / sizeof(upload[0]); ++i) {
        const auto word = i == 1 ? guestAddress : upload[i];
        if (!send(word)) return;
    }

    if (!receive(kDspInit) || !receive(kZeldaHandshake)) return;
    Dsp_Running_Start();
}
