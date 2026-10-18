#include "JSystem/JAudio2/JASAiCtrl.hpp"
#include "JSystem/JAudio2/JASAudioThread.hpp"
#include "JSystem/JAudio2/JASCalc.hpp"
#include "JSystem/JAudio2/JASChannel.hpp"
#include "JSystem/JAudio2/JASCmdStack.hpp"
#include "JSystem/JAudio2/JASCriticalSection.hpp"
#include "JSystem/JAudio2/JASDSPChannel.hpp"
#include "JSystem/JAudio2/JASDSPInterface.hpp"
#include "JSystem/JAudio2/JASHeapCtrl.hpp"
#include "JSystem/JAudio2/JASLfo.hpp"
#include "JSystem/JAudio2/JASProbe.hpp"
#include "JSystem/JAudio2/JASReport.hpp"
#include "JSystem/JKernel/JKRSolidHeap.hpp"


// #include <ai.h>
// #include <os.h>
#include <stdint.h>

void JASDriver::updateDSP() {
    JASProbe::start(3, "SFR-UPDATE");
    JASDsp::invalChannelAll();

    JASPortCmd::execAllCommand();
    DSPSyncCallback();
    static u32 old_time = 0;
    static u32 history[10] = {0x000F4240};
    u32 r28 = OSGetTick();
    u32 r27 = r28 - old_time;
    old_time = r28;
    u32 subFrame = getSubFrames();
    int r26 = JASAudioThread::getDSPSyncCount();
    history[subFrame - r26] = r27;
    if (subFrame != r26 && f32(history[0]) / r27 < 1.1f) {
        JASReport("kill DSP channel");
        JASDSPChannel::killActiveChannel();
    }
    JASChannel::receiveBankDisposeMsg();
    JASDSPChannel::updateAll();

    subframeCallback();
    f32 freeRun = 32028.5f / getDacRate();
    JASLfo::updateFreeRun(freeRun);
    JASProbe::stop(3);
    sSubFrameCounter++;
}
