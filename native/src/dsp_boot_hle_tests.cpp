#include <Native/DspMailbox.hpp>

#include <revolution/dsp.h>

#include <cassert>
#include <cstdio>

void DspBoot(void (*requestCallback)(void*));

namespace {
int initializedWork = 0;
int running = 0;
}

void DspInitWork() {
    ++initializedWork;
}

void Dsp_Running_Start() {
    running = 1;
}

int Dsp_Running_Check() {
    return running;
}

int main() {
    assert(Dsp_Running_Check() == 0);
    DspBoot(nullptr);
    assert(initializedWork == 1);
    assert(Dsp_Running_Check() == 1);
    assert(DSPCheckMailToDSP() == 0);
    assert(DSPCheckMailFromDSP() == 0);

    // The host boot provider must not claim to execute later Zelda audio work.
    DSPSendMailToDSP(0x81000040u);
    assert(DSPCheckMailFromDSP() == 0);
    std::puts("PASS host DspBoot validated SMG ROM startup mails; post-start audio work is not acknowledged");
}
