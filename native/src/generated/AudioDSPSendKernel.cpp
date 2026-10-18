#include <JSystem/JAudio2/dsptask.hpp>
#include <Native/DspMailbox.hpp>
#include <revolution/dsp.h>
#include <revolution/os.h>
int Dsp_Running_Check(); int DspStartWork(u32,void(*)(u16));
void DspFinishWork(u16);
int DSPSendCommands2(u32* pMessages, u32 param_2, void (*pCallback)(u16)) {
    s32 i;
    BOOL interruptFlag;
    s32 startWorkStatus=0;

    BOOL firstWarning = 1;
    while (Dsp_Running_Check() == 0) {
        if (firstWarning) {
            OSReport("Warning:まだブートしてません\n");
        }

        firstWarning = 0;
    }

    u16 completionToken = 0;
    const s32 hostAudioResult =
        nativeDspProcessHostAudioCommand(pMessages, param_2, &completionToken);
    if (hostAudioResult != 0) {
        if (hostAudioResult < 0) {
            OSReport("Native DSP HLE rejected command batch (%d)\n", hostAudioResult);
            return -1;
        }

        interruptFlag = OSDisableInterrupts();
        if (pCallback != NULL) {
            startWorkStatus = DspStartWork(pMessages[0], pCallback);
            if (!startWorkStatus) {
                OSRestoreInterrupts(interruptFlag);
                return -1;
            }
            DspFinishWork(completionToken);
        }
        OSRestoreInterrupts(interruptFlag);
        return startWorkStatus;
    }

    interruptFlag = OSDisableInterrupts();
    if (DSPCheckMailToDSP()) {
        OSReport("エラー::DSPがメッセージを受け取っていません\n");
        OSRestoreInterrupts(interruptFlag);
        return -1;
    }

    if (pCallback != NULL) {
        startWorkStatus = DspStartWork(pMessages[0], pCallback);
        if (!startWorkStatus) { OSRestoreInterrupts(interruptFlag); return -1; }
    }
    DSPSendMailToDSP(param_2);
    DSPAssertInt();
    while (DSPCheckMailToDSP() != 0) {
    }

    if (param_2 == 0) {
        param_2 = 1;
    }



    for (i = 0; i < param_2; i++) {
        DSPSendMailToDSP(pMessages[i]);
        while (DSPCheckMailToDSP() != 0) {
        }
    }

    OSRestoreInterrupts(interruptFlag);
    return startWorkStatus;
}

