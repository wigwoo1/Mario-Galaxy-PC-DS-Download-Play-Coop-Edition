#include <JSystem/JAudio2/dsptask.hpp>
#include <revolution/os.h>
typedef struct {
    /* 0x00 */ u16 field_0x0;
    /* 0x02 */ u16 field_0x2;
    /* 0x04 */ void (*field_0x4)(u16);
} TaskWorkStruct;

static TaskWorkStruct taskwork[16];

void DspInitWork() {
    for (int i = 0; i < 16; i++) {
        taskwork[i].field_0x4 = NULL;
    }
}

static u32 taskreadp;

static u32 taskwritep;

int DspStartWork(u32 param_0, void (*pCallback)(u16)) {
    u32 taskWritePrev = taskwritep;
    u32 writeVal = ((taskWritePrev + 1) & 0xf);
    if (writeVal == taskreadp) {
        OSReport("DSP-Task Buffer is full ( %d, %d)\n", taskwritep, taskreadp);
        return 0;
    }

    taskwritep = writeVal;
    taskwork[taskWritePrev].field_0x0 = param_0 >> 0x10;
    taskwork[taskWritePrev].field_0x4 = pCallback;
    return taskWritePrev + 1;
}

void DspFinishWork(u16 param_0) {
    if (param_0 != taskwork[taskreadp].field_0x0) {
        OSReport("Error:: Not finish task\n");
    } else {
        if (taskwork[taskreadp].field_0x4) {
            taskwork[taskreadp].field_0x4(taskreadp);
        }

        taskreadp = (taskreadp + 1) & 0xf;
    }
}

