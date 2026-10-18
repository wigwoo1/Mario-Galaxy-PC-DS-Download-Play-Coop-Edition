#include <Native/DspAddressSpace.hpp>
#include <Native/DspMailbox.hpp>
#include <Native/DspUcodeImage.hpp>

#include <revolution/dsp.h>

#include <cassert>
#include <cstdio>

int main() {
    DSPInit();
    assert(DSPCheckMailToDSP() == 0);
    assert(DSPCheckMailFromDSP() == 0);

    const auto* image = Native::originalDspUcodeImage();
    const auto imageBytes = Native::originalDspUcodeImageBytes();
    assert(nativeDspBeginRomTask(image, imageBytes));
    const auto guestAddress = Native::nativeDspAddressSpace().addressOf(
        image, imageBytes);
    assert(guestAddress != 0);

    assert(DSPCheckMailFromDSP() == 1);
    assert(DSPReadMailFromDSP() == 0x8071FEEDu);
    assert(DSPCheckMailFromDSP() == 0);

    const u32 upload[] = {
        0x80F3A001u, guestAddress, 0x80F3C002u, 0,
        0x80F3A002u, 0x1F00u, 0x80F3B002u, 0,
        0x80F3D001u, 0,
    };
    for (const auto mail : upload) {
        DSPSendMailToDSP(mail);
        assert(DSPCheckMailToDSP() == 0);
    }

    assert(DSPCheckMailFromDSP() == 1);
    assert(DSPReadMailFromDSP() == 0xDCD10000u);
    assert(DSPCheckMailFromDSP() == 1);
    assert(DSPReadMailFromDSP() == 0xF3551111u);
    assert(DSPCheckMailFromDSP() == 0);

    DSPInit();
    assert(nativeDspBeginRomTask(image, imageBytes));
    assert(DSPReadMailFromDSP() == 0x8071FEEDu);
    DSPSendMailToDSP(0xDEADBEEFu);
    assert(DSPCheckMailToDSP() == 0);
    assert(DSPCheckMailFromDSP() == 0); // invalid mail must not fake readiness

    DSPInit();
    assert(!nativeDspBeginRomTask(image, imageBytes - 1));
    assert(DSPCheckMailFromDSP() == 0);
    std::puts("PASS SMG ROM HLE over native DSP mailboxes: ordered upload, queued startup replies and invalid-mail rejection");
}
