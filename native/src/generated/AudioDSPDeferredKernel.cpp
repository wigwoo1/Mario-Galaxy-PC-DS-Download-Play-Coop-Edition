#include <JSystem/JAudio2/dspproc.hpp>
#include <revolution/os.h>
static volatile u8 DspRunningStatus; static u8 lbl_806B75B9;
static u32 sync_stack[5];

void DsyncFrame2(u32 param_0, u32 param_1, u32 param_2) {
    if (DspRunningStatus != 1) {
        OSReport("Yield中です\n");
        sync_stack[0] = param_0;
        lbl_806B75B9 = 1;
        sync_stack[1] = param_1;
        sync_stack[2] = param_2;
        return;
    }

    DsyncFrame2ch(param_0, param_1, param_2);
    lbl_806B75B9 = 0;
}

void DsyncFrame3(u32 param_0, u32 param_1, u32 param_2, u32 param_3, u32 param_4) {
    if (DspRunningStatus != 1) {
        sync_stack[0] = param_0;
        lbl_806B75B9 = 2;
        sync_stack[1] = param_1;
        sync_stack[2] = param_2;
        sync_stack[3] = param_3;
        sync_stack[4] = param_4;
        return;
    }

    DsyncFrame4ch(param_0, param_1, param_2, param_3, param_4);
    lbl_806B75B9 = 0;
}

void Dsp_Update_Request() {
    switch (lbl_806B75B9) {
    case 0:
        break;
    case 1:
        DsyncFrame2(sync_stack[0], sync_stack[1], sync_stack[2]);
        break;
    case 2:
        DsyncFrame3(sync_stack[0], sync_stack[1], sync_stack[2], sync_stack[3], sync_stack[4]);
        break;
    }
}

int Dsp_Running_Check() {
    return DspRunningStatus == 1 ? 1 : 0;
}

void Dsp_Running_Start() {
    DspRunningStatus = 1;
}

