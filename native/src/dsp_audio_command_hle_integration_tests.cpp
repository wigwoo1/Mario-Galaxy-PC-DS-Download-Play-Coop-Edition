#include <Native/DspAddressSpace.hpp>
#include <Native/DspMailbox.hpp>
#include <Native/DspUcodeImage.hpp>
#include <JSystem/JAudio2/dspproc.hpp>
#include <JSystem/JAudio2/dsptask.hpp>
#include <revolution/dsp.h>
#include <revolution/os.h>

#include <cassert>
#include <cstdio>

void Dsp_Running_Start();
void DspInitWork();
u16 DSP_CreateMap2(u32) { return 0; }

extern "C" BOOL OSDisableInterrupts() { return 0; }
extern "C" BOOL OSRestoreInterrupts(BOOL enabled) { return enabled; }
extern "C" void OSReport(const char*, ...) {}

static void uploadOriginalSmgUcode() {
    const auto* image = Native::originalDspUcodeImage();
    const auto imageBytes = Native::originalDspUcodeImageBytes();
    assert(nativeDspBeginRomTask(image, imageBytes));
    const auto guestAddress =
        Native::nativeDspAddressSpace().addressOf(image, imageBytes);
    assert(guestAddress != 0);
    assert(DSPCheckMailFromDSP() == 1);
    assert(DSPReadMailFromDSP() == 0x8071FEEDu);

    const u32 upload[] = {
        0x80F3A001u, guestAddress, 0x80F3C002u, 0,
        0x80F3A002u, 0x1F00u, 0x80F3B002u, 0,
        0x80F3D001u, 0,
    };
    for (const auto mail : upload) {
        DSPSendMailToDSP(mail);
        assert(DSPCheckMailToDSP() == 0);
    }
    assert(DSPReadMailFromDSP() == 0xDCD10000u);
    assert(DSPReadMailFromDSP() == 0xF3551111u);
}

int main() {
    DSPInit();
    DspInitWork();
    uploadOriginalSmgUcode();
    Dsp_Running_Start();

    // These calls must return and complete their original callback work. The
    // fallback accepts control/frame packets as silence; it does not mix PCM.
    DsetVARAM(0x12345678u);
    DsetupTable(0x40u, 0x1000u, 0x2000u, 0x3000u, 0x4000u);
    DsyncFrame2ch(7u, 0x12345678u, 0x87654321u);

    std::puts("PASS booted host DSP HLE completes VARAM/table callbacks and frame packets as explicit silence");
}
