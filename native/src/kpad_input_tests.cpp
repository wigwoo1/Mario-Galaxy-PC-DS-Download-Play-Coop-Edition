#include <Native/PointerMotion.hpp>
#include <Native/KeyboardKpad.hpp>
#include <Native/KpadInput.hpp>
#include <Native/WpadHost.hpp>
#include <Native/GameInput.hpp>
#include <cassert>
#include <cstdio>
namespace { bool infoCallbackFired=false;int infoCallbackResult=WPAD_ERR_INVALID;int lastConnectResult=WPAD_ERR_INVALID;void onInfo(s32,s32 result){infoCallbackFired=true;infoCallbackResult=result;}void onConnect(s32,s32 result){lastConnectResult=result;} }
namespace {
int extensionCalls=0,extensionResult=WPAD_DEV_UNKNOWN;
int connectReplayCalls=0;
void onReplayConnect(s32 channel,s32 result){++connectReplayCalls;u32 device=WPAD_DEV_UNKNOWN;assert(WPADProbe(channel,&device)==result);}
void onExtension(s32 channel,s32 result){
 ++extensionCalls;extensionResult=result;u32 device=WPAD_DEV_UNKNOWN;
 assert(WPADProbe(channel,&device)==WPAD_ERR_NONE&&device==unsigned(result));
}
}
int main(){
 KPADInit();KPADStatus a{},b{},out[120]{};a.hold=WPAD_BUTTON_A;a.trig=WPAD_BUTTON_A;a.dev_type=WPAD_DEV_FREESTYLE;a.ex_status.fs.stick.x=.75f;
 b=a;b.hold=0;b.trig=0;b.release=WPAD_BUTTON_A;
 assert(nativeSubmitKpadSample(0,a));assert(nativeSubmitKpadSample(0,b));assert(nativeSubmitKpadSample(1,a));
 assert(KPADRead(0,out,120)==2);assert(out[0].release==WPAD_BUTTON_A&&out[1].trig==WPAD_BUTTON_A);assert(out[1].ex_status.fs.stick.x==.75f);
 assert(KPADRead(0,out,120)==0);assert(KPADRead(1,out,1)==1);
 assert(!nativeSubmitKpadSample(-1,a)&&!nativeSubmitKpadSample(4,a));assert(KPADRead(-1,out,120)==0);
 for(unsigned i=0;i<120;++i){a.hold=i;assert(nativeSubmitKpadSample(2,a));}
 assert(!nativeSubmitKpadSample(2,b));assert(KPADRead(2,nullptr,120)==0&&KPADRead(2,out,0)==0);
 assert(KPADRead(2,out,2)==2&&out[0].hold==119&&out[1].hold==118);assert(KPADRead(2,out,120)==118&&out[0].hold==117&&out[117].hold==0);
 assert(nativeSubmitKpadSample(3,b));KPADReset();assert(KPADRead(3,out,120)==0);
 KPADSetBtnRepeat(0,.5f,.25f);a={};a.hold=WPAD_BUTTON_A;a.wpad_err=WPAD_ERR_NONE;
 assert(nativeSubmitKpadState(0,a,0));assert(KPADRead(0,out,1)==1&&out[0].trig==WPAD_BUTTON_A);
 assert(nativeSubmitKpadState(0,a,.49));assert(KPADRead(0,out,1)==1&&out[0].trig==0&&!(out[0].hold&KPAD_BUTTON_RPT));
 assert(nativeSubmitKpadState(0,a,.5));assert(KPADRead(0,out,1)==1&&(out[0].hold&KPAD_BUTTON_RPT));
 assert(nativeSubmitKpadState(0,a,.6));assert(KPADRead(0,out,1)==1&&!(out[0].hold&KPAD_BUTTON_RPT));
 assert(!nativeSubmitKpadState(0,a,.59));a.hold=0;assert(nativeSubmitKpadState(0,a,.75));assert(KPADRead(0,out,1)==1&&out[0].release==WPAD_BUTTON_A);
 a.hold=WPAD_BUTTON_Z;assert(nativeSubmitKpadState(1,a,1));assert(KPADRead(1,out,1)==1&&out[0].trig==WPAD_BUTTON_Z);
 a.wpad_err=WPAD_ERR_NO_CONTROLLER;assert(nativeSubmitKpadState(1,a,2));assert(KPADRead(1,out,1)==1&&out[0].release==WPAD_BUTTON_Z&&out[0].hold==0);
 KPADReset();a.wpad_err=WPAD_ERR_NONE;assert(nativeSubmitKpadState(1,a,0));assert(KPADRead(1,out,1)==1&&out[0].trig==WPAD_BUTTON_Z);
 KPADReset();a.hold=0;
 for(int i=0;i<120;++i)assert(nativeSubmitKpadState(2,a,i));
 a.hold=WPAD_BUTTON_B;assert(!nativeSubmitKpadState(2,a,120));assert(KPADRead(2,out,120)==120);
 assert(nativeSubmitKpadState(2,a,120));assert(KPADRead(2,out,1)==1&&out[0].trig==WPAD_BUTTON_B);
 NativeKeyboardState keyboard{};keyboard.forward=true;keyboard.right=true;keyboard.jump=true;keyboard.crouch=true;keyboard.pointerValid=true;keyboard.pointerX=.25f;keyboard.pointerY=-.5f;
 auto mapped=nativeMapKeyboard(keyboard);assert(std::fabs(mapped.ex_status.fs.stick.x-.70710678f)<.00001f);assert(mapped.ex_status.fs.stick.x==mapped.ex_status.fs.stick.y);
 assert(mapped.hold==(WPAD_BUTTON_A|WPAD_BUTTON_Z)&&mapped.dpd_valid_fg==2&&mapped.pos.x==.25f&&mapped.pos.y==-.5f);
 keyboard={};keyboard.pointerTrigger=true;mapped=nativeMapKeyboard(keyboard);
 assert(mapped.hold==WPAD_BUTTON_B);assert(nativeSubmitKpadState(0,mapped,1.0));
 assert(KPADRead(0,out,1)==1&&out[0].trig==WPAD_BUTTON_B&&out[0].hold==WPAD_BUTTON_B);
 keyboard.pointerTrigger=false;mapped=nativeMapKeyboard(keyboard);assert(nativeSubmitKpadState(0,mapped,1.01));
 assert(KPADRead(0,out,1)==1&&out[0].release==WPAD_BUTTON_B&&out[0].hold==0);
 NativeKeyboardState shake{};auto resting=nativeMapKeyboard(shake);assert(resting.ex_status.fs.acc.x==0.0f);
 shake.shake=true;auto impulse=nativeMapKeyboard(shake);assert(impulse.ex_status.fs.acc.x==2.0f&&impulse.ex_status.fs.acc.y==1.0f);
 bool shakeHeld=false;assert(nativeShakeImpulse(true,shakeHeld));assert(!nativeShakeImpulse(true,shakeHeld));
 assert(!nativeShakeImpulse(false,shakeHeld)&&nativeShakeImpulse(true,shakeHeld));
 KPADSetPosParam(0,.1f,.2f);KPADSetHoriParam(0,.3f,.4f);KPADSetDistParam(0,.5f,.6f);KPADSetAccParam(0,.7f,.8f);KPADSetSensorHeight(0,1.2f);
 NativeKpadCalibration calibration{};assert(nativeReadKpadCalibration(0,&calibration));
 assert(calibration.posRadius==.1f&&calibration.posSensitivity==.2f&&calibration.horizonRadius==.3f&&calibration.horizonSensitivity==.4f);
 assert(calibration.distanceRadius==.5f&&calibration.distanceSensitivity==.6f&&calibration.accelerationRadius==.7f&&calibration.accelerationSensitivity==.8f&&calibration.sensorHeight==1.2f);
 assert(!nativeReadKpadCalibration(-1,&calibration)&&!nativeReadKpadCalibration(0,nullptr));
 keyboard.left=true;keyboard.back=true;keyboard.right=true;keyboard.forward=true;mapped=nativeMapKeyboard(keyboard);assert(mapped.ex_status.fs.stick.x==0&&mapped.ex_status.fs.stick.y==0);
 mapped=nativeMapKeyboard({});assert(mapped.hold==0&&mapped.dpd_valid_fg==0&&mapped.acc.y==1);
 NativeKeyboardState mouse{};
 assert(nativeMapClientPointer(mouse,0,0,640,480));
 assert(mouse.pointerValid&&mouse.pointerX==-1&&mouse.pointerY==-1);
 assert(nativeMapClientPointer(mouse,320,240,640,480));
 assert(mouse.pointerX==0&&mouse.pointerY==0);
 assert(nativeMapClientPointer(mouse,639,479,640,480));
 auto cursor=nativeMapKeyboard(mouse);
 assert(std::fabs((cursor.pos.x+1)*320-639)<.0001f);
 assert(std::fabs((cursor.pos.y+1)*240-479)<.0001f);
 assert(nativeMapClientPointer(mouse,640,480,640,480)&&!mouse.pointerValid);
 assert(nativeMapClientPointer(mouse,-1,0,640,480)&&!mouse.pointerValid);
 assert(!nativeMapClientPointer(mouse,1,1,0,480)&&!mouse.pointerValid);
 // A 4:3 game image centered in a wider client area leaves invalid side bars.
 assert(nativeMapClientPointer(mouse,80,240,800,480,80,0,640,480));
 assert(mouse.pointerValid&&mouse.pointerX==-1&&mouse.pointerY==0);
 assert(nativeMapClientPointer(mouse,400,240,800,480,80,0,640,480));
 assert(mouse.pointerValid&&mouse.pointerX==0&&mouse.pointerY==0);
 assert(nativeMapClientPointer(mouse,719,479,800,480,80,0,640,480));
 assert(mouse.pointerValid&&mouse.pointerX<1&&mouse.pointerY<1);
 assert(nativeMapClientPointer(mouse,79,240,800,480,80,0,640,480)&&!mouse.pointerValid);
 assert(nativeMapClientPointer(mouse,720,240,800,480,80,0,640,480)&&!mouse.pointerValid);
 assert(!nativeMapClientPointer(mouse,1,1,800,480,790,0,20,480)&&!mouse.pointerValid);
 std::puts("PASS mouse client coordinates: top-left/center/bottom-right/original screen roundtrip/outside/zero extent");
 NativePointerMotion motion;KPADStatus moving{};moving.dpd_valid_fg=2;moving.pos={0,0};
 motion.prepare(moving);assert(moving.speed==0);motion.commit(moving);
 moving.pos={.3f,-.4f};motion.prepare(moving);
 assert(std::fabs(moving.speed-.5f)<.00001f&&moving.vec.x==.3f&&moving.vec.y==-.4f);
 // Not committing a rejected sample preserves the previous accepted origin.
 moving.pos={0,.5f};motion.prepare(moving);assert(moving.speed==.5f);
 motion.commit(moving);motion.prepare(moving);assert(moving.speed==0);
 moving.dpd_valid_fg=0;motion.prepare(moving);motion.commit(moving);
 moving.dpd_valid_fg=2;moving.pos={1,1};motion.prepare(moving);assert(moving.speed==0);
 std::puts("PASS native pointer motion: first sample/displacement/rejected sample origin/stationary/loss/reacquisition");
 WPADInit();u32 deviceType=WPAD_DEV_UNKNOWN;assert(WPADProbe(WPAD_CHAN0,&deviceType)==WPAD_ERR_NONE&&deviceType==WPAD_DEV_FREESTYLE);
 assert(WPADProbe(WPAD_CHAN1,&deviceType)==WPAD_ERR_NO_CONTROLLER&&deviceType==WPAD_DEV_NOT_FOUND);
 WPADInfo info{};assert(WPADGetInfoAsync(WPAD_CHAN0,&info,onInfo)==WPAD_ERR_NONE);assert(!infoCallbackFired);
 nativeDispatchWpadCallbacks();assert(infoCallbackFired&&infoCallbackResult==WPAD_ERR_NONE&&info.dpd&&info.battery==WPAD_BATTERY_LEVEL_MAX);
 assert(WPADSetConnectCallback(WPAD_CHAN0,onConnect)==nullptr);
 WPADInfo canceledInfo{};infoCallbackFired=false;assert(WPADGetInfoAsync(WPAD_CHAN0,&canceledInfo,onInfo)==WPAD_ERR_NONE);
 nativeSetHostWpadConnected(false);assert(WPADProbe(WPAD_CHAN0,&deviceType)==WPAD_ERR_NO_CONTROLLER&&lastConnectResult==WPAD_ERR_NO_CONTROLLER);
 nativeDispatchWpadCallbacks();assert(infoCallbackFired&&infoCallbackResult==WPAD_ERR_NO_CONTROLLER&&!canceledInfo.dpd);
 infoCallbackFired=false;assert(WPADGetInfoAsync(WPAD_CHAN0,&canceledInfo,onInfo)==WPAD_ERR_NO_CONTROLLER&&infoCallbackFired&&infoCallbackResult==WPAD_ERR_NO_CONTROLLER);
 nativeSetHostWpadConnected(true);assert(WPADProbe(WPAD_CHAN0,&deviceType)==WPAD_ERR_NONE&&lastConnectResult==WPAD_ERR_NONE);
 nativeBindGameInputWindow(nullptr);nativePumpGameInputFrame();assert(WPADProbe(WPAD_CHAN0,&deviceType)==WPAD_ERR_NO_CONTROLLER&&lastConnectResult==WPAD_ERR_NO_CONTROLLER);
 std::puts("PASS virtual WPAD channel 0 discovery, deferred info and host-window connect transitions");
 WPADInit();connectReplayCalls=0;
 assert(WPADSetConnectCallback(0,onReplayConnect)==nullptr);
 assert(connectReplayCalls==0);nativeDispatchWpadCallbacks();assert(connectReplayCalls==1);
 nativeDispatchWpadCallbacks();assert(connectReplayCalls==1);
 assert(WPADSetConnectCallback(0,onReplayConnect)==onReplayConnect);
 assert(WPADSetConnectCallback(0,nullptr)==onReplayConnect);
 nativeDispatchWpadCallbacks();assert(connectReplayCalls==1);
 assert(WPADSetConnectCallback(0,onReplayConnect)==nullptr);
 WPADDisconnect(0);assert(connectReplayCalls==2);
 nativeDispatchWpadCallbacks();assert(connectReplayCalls==2);
 nativeSetHostWpadConnected(true);assert(connectReplayCalls==3);
 nativeDispatchWpadCallbacks();assert(connectReplayCalls==3);
 std::puts("PASS already-connected virtual WPAD discovery, callback replay/cancellation and reconnect without duplicate notification");
 WPADInit();extensionCalls=0;
 assert(WPADSetExtensionCallback(0,onExtension)==nullptr);
 assert(extensionCalls==0);nativeDispatchWpadCallbacks();
 assert(extensionCalls==1&&extensionResult==WPAD_DEV_FREESTYLE);
 nativeDispatchWpadCallbacks();assert(extensionCalls==1);
 assert(WPADSetExtensionCallback(0,onExtension)==onExtension);
 assert(WPADSetExtensionCallback(0,nullptr)==onExtension);
 nativeDispatchWpadCallbacks();assert(extensionCalls==1);
 assert(WPADSetExtensionCallback(1,onExtension)==nullptr);
 assert(WPADSetExtensionCallback(4,onExtension)==nullptr);
 nativeDispatchWpadCallbacks();assert(extensionCalls==1);
 assert(WPADSetExtensionCallback(0,onExtension)==nullptr);
 WPADDisconnect(0);nativeDispatchWpadCallbacks();assert(extensionCalls==1);
 nativeSetHostWpadConnected(true);assert(extensionCalls==1);
 nativeDispatchWpadCallbacks();assert(extensionCalls==2&&extensionResult==WPAD_DEV_FREESTYLE);
 WPADInfo extensionInfo{};infoCallbackFired=false;
 assert(WPADGetInfoAsync(0,&extensionInfo,onInfo)==WPAD_ERR_NONE);
 nativeDispatchWpadCallbacks();assert(infoCallbackFired&&extensionInfo.attach==1);
 assert(WPADSetExtensionCallback(0,nullptr)==onExtension);
 nativeSetHostWpadConnected(false);nativeSetHostWpadConnected(true);
 nativeDispatchWpadCallbacks();assert(extensionCalls==2);
 std::puts("PASS WPAD extension discovery, deferred/reentrant-safe delivery, unsubscribe/cancellation, reconnect and attached-info consistency");
 std::puts("PASS KPAD channel isolation, newest-first edges, analog data, capacity, partial reads and reset");
}







