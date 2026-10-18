#include <Native/VideoBinding.hpp>
#include <Native/VideoRetraceScheduler.hpp>
#include <Native/GameInput.hpp>
#include <cassert>
#include <cstdio>
int main(){
 bool unbound=false;try{VIInit();}catch(const std::logic_error&){unbound=true;}assert(unbound);
 {
 Native::VideoInterface state;Native::VideoBinding binding(state);
 Native::VideoRetraceScheduler scheduler(state,std::chrono::milliseconds(10));
 VIInit();GXRenderModeObj mode{};mode.fbWidth=608;mode.efbHeight=448;mode.xfbHeight=448;mode.viWidth=640;mode.viHeight=448;mode.viTVmode=VITVMode(2);
 VIConfigure(&mode);int buffer;VISetNextFrameBuffer(&buffer);VISetBlack(0);VIFlush();assert(VIGetNextFrameBuffer()==&buffer);
 VIWaitForRetrace();assert(VIGetCurrentFrameBuffer()==&buffer&&VIGetRetraceCount()>0&&VIGetScanMode()==2&&!state.current().black);
 scheduler.stop();
 const u32 beforeDimming=VIGetDimmingCount();assert(beforeDimming==18000-VIGetRetraceCount());
 state.retrace();assert(VIGetDimmingCount()==beforeDimming-1);
 assert(VIEnableDimming(0)==1);state.retrace();assert(VIGetDimmingCount()==beforeDimming-1);
 assert(VIEnableDimming(1)==0);state.retrace();assert(VIGetDimmingCount()==beforeDimming-2);
 nativeNotifyVideoActivity();assert(VIGetDimmingCount()==18000);
 bool nested=false;try{Native::VideoBinding other(state);}catch(const std::logic_error&){nested=true;}assert(nested);
 }
 {
 Native::VideoInterface pal;Native::VideoBinding binding(pal);GXRenderModeObj mode{};mode.fbWidth=608;mode.efbHeight=448;mode.xfbHeight=448;mode.viWidth=640;mode.viHeight=448;mode.viTVmode=VI_TVMODE_PAL_PROG;
 VIConfigure(&mode);assert(VIGetDimmingCount()==15000);
 }
 bool disposed=false;try{VIGetRetraceCount();}catch(const std::logic_error&){disposed=true;}assert(disposed);
 std::puts("PASS SDK VI binding: retrace, framebuffer latch, dimming enable/count/activity reset and lifetime");
}

