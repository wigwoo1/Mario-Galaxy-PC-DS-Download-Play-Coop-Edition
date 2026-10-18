#define main originalHeapFixtureEntry
#include "unit_heap_live_tests.cpp"
#undef main
#include <Native/VideoBinding.hpp>
#include <Native/VideoRetraceScheduler.hpp>
#include <Native/DrawCompletion.hpp>
#include <JSystem/JUtility/JUTVideo.hpp>
int main(){
 alignas(32) static unsigned char storage[1024*1024]{};
 JKRExpHeap heap(storage,sizeof(storage),nullptr,false);JKRHeap::sRootHeap=&heap;JKRHeap::sSystemHeap=&heap;heap.becomeCurrentHeap();
 Native::VideoInterface state;Native::VideoBinding binding(state);
 Native::VideoRetraceScheduler scheduler(state,std::chrono::milliseconds(20));
 GXRenderModeObj mode{};mode.fbWidth=608;mode.efbHeight=448;mode.xfbHeight=448;mode.viWidth=640;mode.viHeight=448;mode.viTVmode=VITVMode(2);
 auto* manager=JUTVideo::createManager(&mode);assert(manager&&JUTVideo::getManager()==manager&&manager->getEfbHeight()==448);
 VIWaitForRetrace();scheduler.stop();scheduler.checkFailure();
 assert(VIGetRetraceCount()>=3&&state.current().configured);
 Native::dispatchDrawDoneCallback();
 GXSetDrawDoneCallback(nullptr);JUTVideo::destroyManager();assert(JUTVideo::getManager()==nullptr);
 std::puts("PASS original JUTVideo startup: native constructor retraces, live render mode, callbacks and teardown");
}
