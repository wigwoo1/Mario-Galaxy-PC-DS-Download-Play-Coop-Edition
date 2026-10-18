#include <JSystem/JAudio2/JASDspInterface.hpp>
#include <JSystem/JAudio2/dspproc.hpp>
#include <JSystem/JAudio2/dsptask.hpp>
void JASDsp::releaseHalt(u32 param_0) {
    DSPReleaseHalt2(param_0);
}
void JASDsp::finishWork(u16 param_0) {
    DspFinishWork(param_0);
}

