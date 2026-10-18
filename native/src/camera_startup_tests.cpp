#include <Native/DisplayConfig.hpp>
#define main originalHeapFixtureEntry
#include "unit_heap_live_tests.cpp"
#undef main
#include <Native/VideoBinding.hpp>
#include <Native/VideoRetraceScheduler.hpp>
#include <Native/DrawCompletion.hpp>
#include <JSystem/JUtility/JUTVideo.hpp>
#include <Game/Camera/CameraContext.hpp>
#include <Game/NameObj/NameObjHolder.hpp>
#include <Game/NameObj/NameObjRegister.hpp>
#include <Game/Util/SingletonHolder.hpp>
#include <cmath>
int main(){
 alignas(32) static unsigned char storage[1024*1024]{};
 JKRExpHeap heap(storage,sizeof(storage),nullptr,false);JKRHeap::sRootHeap=&heap;JKRHeap::sSystemHeap=&heap;heap.becomeCurrentHeap();
 Native::VideoInterface state;Native::VideoBinding binding(state);
 Native::VideoRetraceScheduler scheduler(state,std::chrono::milliseconds(20));
 GXRenderModeObj mode{};mode.fbWidth=608;mode.efbHeight=448;mode.xfbHeight=448;mode.viWidth=640;mode.viHeight=448;mode.viTVmode=VITVMode(2);
 auto* manager=JUTVideo::createManager(&mode);assert(manager&&JUTVideo::getManager()==manager&&manager->getEfbHeight()==448);
 VIWaitForRetrace();scheduler.stop();scheduler.checkFailure();
 assert(VIGetRetraceCount()>=3&&state.current().configured);
 NameObjHolder names(8);SingletonHolder<NameObjRegister>::init();SingletonHolder<NameObjRegister>::get()->setCurrentHolder(&names);
 {
 auto display=Native::displayConfig();display.widescreen=false;Native::setDisplayConfig(display);CameraContext camera;assert(camera.mNearZ==100&&camera.mFarZ==800000&&camera.mFovy==45);
 assert(std::fabs(camera.mAspectRatio-608.f/448)<1e-6f);
 assert(camera.getViewMtx().mMtx[2][3]==-3000&&camera.getInvViewMtx().mMtx[2][3]==3000);
 TPos3f authored;authored.identity();
 authored.mMtx[0][0]=0;authored.mMtx[0][1]=-1;
 authored.mMtx[1][0]=1;authored.mMtx[1][1]=0;
 authored.mMtx[0][3]=10;authored.mMtx[1][3]=20;authored.mMtx[2][3]=30;
 camera.setViewMtx(authored,false,false,TVec3f(0,0,0));
 const float inverse[3][4]={{0,1,0,-20},{-1,0,0,10},{0,0,1,-30}};
 for(int row=0;row<3;row++)for(int col=0;col<4;col++){
 assert(std::fabs(camera.getViewMtx().mMtx[row][col]-authored.mMtx[row][col])<1e-6f);
 assert(std::fabs(camera.getInvViewMtx().mMtx[row][col]-inverse[row][col])<1e-6f);
 }
 camera.setNearZ(50);camera.setFarZ(10000);camera.setFovy(60);
 const auto projection=camera.mProjection;camera.setShakeOffset(.125f,-.25f);
 for(int col=0;col<4;++col){assert(std::fabs(camera.mProjection.mMtx[0][col]-(projection.mMtx[0][col]+.125f*projection.mMtx[3][col]))<1e-6f);assert(std::fabs(camera.mProjection.mMtx[1][col]-(projection.mMtx[1][col]-.25f*projection.mMtx[3][col]))<1e-6f);}
 }
 auto restore=Native::displayConfig();restore.widescreen=true;Native::setDisplayConfig(restore);
 std::puts("PASS original CameraContext construction: live EFB aspect, inverse view, clip/FOV updates and shake projection");
 names.clearArray();SingletonHolder<NameObjRegister>::get()->setCurrentHolder(nullptr);
 Native::dispatchDrawDoneCallback();
 GXSetDrawDoneCallback(nullptr);JUTVideo::destroyManager();assert(JUTVideo::getManager()==nullptr);
 std::puts("PASS original JUTVideo startup: native constructor retraces, live render mode, callbacks and teardown");
}



