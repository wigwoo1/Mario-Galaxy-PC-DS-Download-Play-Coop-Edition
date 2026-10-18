#include <Native/PointerMotion.hpp>
#include <Game/System/WPadPointer.hpp>
#include <Game/Util/GamePadUtil.hpp>
#include <Game/System/WPadStick.hpp>
#include <Native/KeyboardKpad.hpp>
#include <Game/System/WPad.hpp>
#include <Game/System/WPadAcceleration.hpp>
#include <Game/System/WPadHVSwing.hpp>
#include <Game/System/WPadHolder.hpp>
#include <Game/System/WPadButton.hpp>
#include <cassert>
#include <cstdio>
// Fixture construction only: full WPad device services are not started here.
WPad::WPad(s32 channel):mChannel(channel),mReadInfo(nullptr){}
WPadReadDataInfo::WPadReadDataInfo():mStatusArray(new KPADStatus[120]{}),mValidStatusCount(0){}
static WPad* fixturePad=nullptr; namespace MR { WPad* getWPad(s32 channel){assert(channel==0);return fixturePad;} }
namespace MR { f32 getSubPadStickX(s32){return 0.0f;} f32 getSubPadStickY(s32){return 0.0f;} }
namespace JMathInlineVEC { f32 PSVECSquareDistance(const Vec* a,const Vec* b){const f32 x=a->x-b->x,y=a->y-b->y,z=a->z-b->z;return x*x+y*y+z*z;} }
int main(){
 KPADInit();WPad pad(0);WPadReadDataInfo read;pad.mReadInfo=&read;WPadButton button(&pad);pad.mButton=&button;fixturePad=&pad;pad.mIsConnected=false;assert(!MR::isConnectedWPad(0));pad.mIsConnected=true;assert(MR::isConnectedWPad(0));
 auto update=[&](const KPADStatus& state,double time){assert(nativeSubmitKpadState(0,state,time));read.mValidStatusCount=KPADRead(0,read.mStatusArray,120);button.update();};
 NativeKeyboardState keyboard{};keyboard.jump=true;keyboard.crouch=true;
 auto state=nativeMapKeyboard(keyboard);update(state,0);
 assert(MR::testCorePadTriggerA(0));
 assert(button.testButtonA()&&button.testButtonZ()&&button.testTriggerA()&&button.testTriggerZ()&&button.isChangeAnyState());
 update(state,.1);assert(!MR::testCorePadTriggerA(0));
 assert(button.testButtonA()&&!button.testTriggerA()&&!button.isChangeAnyState());
 update(state,.5);assert((read.mStatusArray[0].hold&KPAD_BUTTON_RPT)&&!button.testTriggerA());
 keyboard={};update(nativeMapKeyboard(keyboard),.6);assert(!MR::testCorePadTriggerA(0));assert(!button.testButtonA()&&!button.testButtonZ()&&button.testReleaseZ());
 read.mValidStatusCount=0;button.update();assert(button.testReleaseZ()); // Original empty-read semantics retain state.
 keyboard.crouch=true;state=nativeMapKeyboard(keyboard);update(state,.7);assert(button.testButtonZ());
 state.wpad_err=WPAD_ERR_NO_CONTROLLER;update(state,.8);assert(!button.testButtonZ()&&button.testReleaseZ()&&!button.testTriggerZ());
 update(state,.9);assert(!button.testReleaseZ()&&!button.isChangeAnyState());
 state.wpad_err=WPAD_ERR_NONE;update(state,1);assert(button.testButtonZ()&&button.testTriggerZ());
 state.wpad_err=WPAD_ERR_BUSY;update(state,1.1);assert(button.testButtonZ()&&!button.testTriggerZ());
 WPadStick stick(&pad);
 auto move=[&](float x,float y,double time){auto sample=nativeMapKeyboard({});sample.ex_status.fs.stick={x,y};update(sample,time);stick.update();};
 move(.2f,0,2);assert(stick.mHold==0&&stick.isChanged());
 move(.21f,0,2.1);assert(stick.mHold==4&&stick.mTrigger==4);
 move(.21f,0,2.2);assert(stick.mTrigger==0&&!stick.isChanged());
 move(-1,-1,2.3);assert(stick.mHold==10&&stick.mTrigger==10&&stick.mRelease==4);
 move(0,0,2.4);assert(stick.mHold==0&&stick.mRelease==10&&!stick.mIsHoldUp&&!stick.mIsHoldDown);
 keyboard={};keyboard.forward=true;keyboard.right=true;update(nativeMapKeyboard(keyboard),2.5);stick.update();
 assert(stick.mHold==5&&stick.mIsTriggerUp&&stick.mIsHoldUp);
 state=nativeMapKeyboard(keyboard);state.wpad_err=WPAD_ERR_NO_CONTROLLER;update(state,2.6);stick.update();assert(stick.mHold==0&&stick.mRelease==5&&stick.mStick.x==0&&stick.mStick.y==0&&!stick.mIsHoldUp);
 update(state,2.7);stick.update();assert(stick.mRelease==0&&!stick.isChanged());
 state.wpad_err=WPAD_ERR_NONE;update(state,2.8);stick.update();assert(stick.mHold==5&&stick.mTrigger==5&&stick.mIsTriggerUp);
 // Exercise the original startup predicate for every accepted remote button.
 double buttonTime=3.0;
 const unsigned coreButtons[]={WPAD_BUTTON_UP,WPAD_BUTTON_DOWN,WPAD_BUTTON_LEFT,WPAD_BUTTON_RIGHT,
 WPAD_BUTTON_A,WPAD_BUTTON_B,WPAD_BUTTON_1,WPAD_BUTTON_2,WPAD_BUTTON_PLUS,WPAD_BUTTON_MINUS};
 for(unsigned bit:coreButtons){
   auto sample=nativeMapKeyboard({});update(sample,buttonTime);buttonTime+=.01;
   sample.hold=bit;update(sample,buttonTime);buttonTime+=.01;
   assert(MR::testCorePadTriggerAnyWithoutHome(0));assert(MR::testPadButtonAnyWithoutHome(0));
   update(sample,buttonTime);buttonTime+=.01;
   assert(!MR::testCorePadTriggerAnyWithoutHome(0));assert(MR::testPadButtonAnyWithoutHome(0));
 }
 auto excluded=nativeMapKeyboard({});update(excluded,buttonTime);buttonTime+=.01;
 excluded.hold=WPAD_BUTTON_HOME;update(excluded,buttonTime);buttonTime+=.01;
 assert(button.testTriggerHome()&&!MR::testCorePadTriggerAnyWithoutHome(0)&&!MR::testPadButtonAnyWithoutHome(0));
 excluded.hold=0;update(excluded,buttonTime);buttonTime+=.01;
 excluded.hold=WPAD_BUTTON_Z;update(excluded,buttonTime);
 assert(button.testTriggerZ()&&!MR::testCorePadTriggerAnyWithoutHome(0)&&MR::testPadButtonAnyWithoutHome(0));
 std::puts("PASS original startup any-button trigger: ten accepted keys, held keys, HOME/Z exclusion");
 buttonTime+=.01;excluded.hold=WPAD_BUTTON_C;update(excluded,buttonTime);
 assert(MR::testSubPadButtonC(0)&&MR::testPadButtonAnyWithoutHome(0));
 buttonTime+=.01;excluded.hold=WPAD_BUTTON_A;update(excluded,buttonTime);
 assert(MR::testCorePadButtonA(0)&&!MR::testCorePadButtonB(0));
 buttonTime+=.01;update(excluded,buttonTime);
 assert(MR::testCorePadButtonA(0)&&!MR::testCorePadTriggerA(0));
 buttonTime+=.01;excluded.wpad_err=WPAD_ERR_NO_CONTROLLER;update(excluded,buttonTime);
 assert(!MR::testCorePadButtonA(0)&&!MR::testPadButtonAnyWithoutHome(0));
 std::puts("PASS original held-button queries: core buttons/C/Z/HOME exclusion/A hold/disconnect");
 // First inject known history for accessor checks; then exercise original update below.
 WPadPointer pointer(&pad);pad.mPointer=&pointer;assert(pointer.mInValidOutScreenTime==0);
 delete[] pointer.mPointingPosArray;delete[] pointer.mHorizonArray;
 TVec2f positions[120]={TVec2f(-1,-1),TVec2f(0,0),TVec2f(1,1)};
 TVec2f horizons[120]={TVec2f(1,0),TVec2f(0,1),TVec2f(-1,0)};
 pointer.mPointingPosArray=positions;pointer.mHorizonArray=horizons;
 pointer.mEnablePastCount=3;pointer.mIsPointInScreen=true;
 TVec2f out(9,9);MR::getCorePadPointingPos(&out,0);assert(out.x==1&&out.y==1);
 assert(MR::getCorePadEnablePastCount(0)==3&&MR::isCorePadPointInScreen(0));
 for(int index=0;index<3;++index){MR::getCorePadPastPointingPos(&out,index,0);assert(out.x==positions[2-index].x&&out.y==positions[2-index].y);}
 pointer.getHorizonVec(&out);assert(out.x==-1&&out.y==0);
 pointer.mIsPointInScreen=false;MR::getCorePadPointingPos(&out,0);assert(out.x==0&&out.y==0);
 pointer.getHorizonVec(&out);assert(out.x==0&&out.y==0&&!MR::isCorePadPointInScreen(0));
 std::puts("PASS original pointer accessors: newest position/history order/horizon/offscreen zero");
 pointer.mInValidOutScreenTime=77;pointer.reset();assert(pointer.mInValidOutScreenTime==0);
 NativeKeyboardState mouse{};mouse.pointerValid=true;mouse.pointerX=.25f;mouse.pointerY=-.5f;
 auto pointing=nativeMapKeyboard(mouse);NativePointerMotion mouseMotion;mouseMotion.prepare(pointing);mouseMotion.commit(pointing);
 for(int i=0;i<5;++i){buttonTime+=.01;update(pointing,buttonTime);pointer.update();assert(!pointer.mIsPointInScreen);}
 buttonTime+=.01;update(pointing,buttonTime);pointer.update();
 mouse.pointerX=.35f;pointing=nativeMapKeyboard(mouse);mouseMotion.prepare(pointing);mouseMotion.commit(pointing);buttonTime+=.01;update(pointing,buttonTime);pointer.update();
 assert(pointer.mIsPointInScreen&&pointer.mEnablePastCount==1&&pointer.mIsPointerMoved);
 pointer.getPointingPos(&out);assert(out.x==.35f&&out.y==-.5f);
 pointing.dpd_valid_fg=0;buttonTime+=.01;update(pointing,buttonTime);pointer.update();
 assert(!pointer.mIsPointInScreen&&pointer.mEnablePastCount==0&&pointer.mLostSensorTime==1&&pointer.mInValidOutScreenTime==1);
 read.mValidStatusCount=0;pointer.update();
 assert(pointer.mEnablePastCount==0&&pointer.mLostSensorTime==0&&!pointer.mIsPointInScreen&&pointer.mInValidOutScreenTime==0);
 std::puts("PASS original pointer update: five-sample acquisition delay/sixth acquisition/lost sample/empty-read reset");
 // Native KPAD returns newest first; original pointer update walks oldest first.
 KPADReset();pointer.reset();
 for(int index=0;index<8;++index){
   auto queued=nativeMapKeyboard({});queued.dpd_valid_fg=2;queued.pos={float(index)*.1f,-float(index)*.1f};
   queued.horizon={float(index),1.0f};queued.dist=float(index);queued.speed=index==6?.01f:0.0f;
   assert(nativeSubmitKpadSample(0,queued));
 }
 read.mValidStatusCount=KPADRead(0,read.mStatusArray,120);assert(read.mValidStatusCount==8);
 pointer.update();assert(pointer.mEnablePastCount==3&&pointer.mIsPointInScreen&&pointer.mIsPointerMoved);
 assert(pointer.mDistDisplay==7);
 for(int index=0;index<3;++index){
   pointer.getPastPointingPos(&out,index);
   assert(out.x==float(7-index)*.1f && out.y==-float(7-index)*.1f);
 }
 pointer.getHorizonVec(&out);assert(out.x==7&&out.y==1);
 KPADReset();pointer.reset();
 for(int index=0;index<120;++index){
   auto queued=nativeMapKeyboard({});queued.dpd_valid_fg=2;queued.pos={float(index),0.0f};queued.speed=0;
   assert(nativeSubmitKpadSample(0,queued));
 }
 read.mValidStatusCount=KPADRead(0,read.mStatusArray,120);pointer.update();
 assert(pointer.mEnablePastCount==115&&!pointer.mIsPointerMoved);
 pointer.getPointingPos(&out);assert(out.x==119);
 pointer.getPastPointingPos(&out,114);assert(out.x==5);
 std::puts("PASS native queued pointer update: chronological history/latest metadata/movement aggregation/full120 sample batch");
 KPADReset();WPadAcceleration acceleration(&pad,1);WPadHVSwing swing(&pad,0);pad.mCorePadAccel=&acceleration;pad.mCorePadSwing=&swing;
 auto feedAcceleration=[&](const NativeKeyboardState& input,double time){auto sample=nativeMapKeyboard(input);assert(nativeSubmitKpadState(0,sample,time));read.mValidStatusCount=KPADRead(0,read.mStatusArray,120);acceleration.update();swing.update();};
 for(int index=0;index<21;++index)feedAcceleration({},100.0+index);
 NativeKeyboardState shakeInput{};shakeInput.shake=true;feedAcceleration(shakeInput,121.0);assert(swing.mIsTriggerSwing);
 shakeInput.shake=false;feedAcceleration(shakeInput,122.0);assert(!swing.mIsTriggerSwing);
 for(int index=0;index<7;++index)feedAcceleration({},123.0+index);
 shakeInput.shake=true;feedAcceleration(shakeInput,130.0);assert(swing.mIsTriggerSwing);
 std::puts("PASS keyboard shake impulse through original WPadAcceleration/WPadHVSwing: one trigger per press with release rearm");
 // The live Windows adapter ORs keyboard and controller sources before edge
 // detection. A source handoff while the other remains held must not retrigger
 // or momentarily release the original Wii shake action.
 WPadAcceleration mergedAcceleration(&pad,1);WPadHVSwing mergedSwing(&pad,0);pad.mCorePadAccel=&mergedAcceleration;pad.mCorePadSwing=&mergedSwing;
 auto feedMerged=[&](const NativeKeyboardState& input,double time){auto sample=nativeMapKeyboard(input);assert(nativeSubmitKpadState(0,sample,time));read.mValidStatusCount=KPADRead(0,read.mStatusArray,120);mergedAcceleration.update();mergedSwing.update();};
 for(int index=0;index<21;++index)feedMerged({},200.0+index);
 bool shakeKey=false,shakeController=false;
 auto feedShakeSources=[&](double time){NativeKeyboardState sources{};sources.shake=shakeKey||shakeController;feedMerged(sources,time);};
 shakeKey=true;feedShakeSources(221.0);assert(mergedSwing.mIsTriggerSwing);
 shakeController=true;feedShakeSources(221.1);assert(!mergedSwing.mIsTriggerSwing);
 shakeKey=false;feedShakeSources(221.2);assert(!mergedSwing.mIsTriggerSwing);
 shakeController=false;feedShakeSources(221.3);assert(!mergedSwing.mIsTriggerSwing);
 for(int index=0;index<7;++index)feedMerged({},222.0+index);
 shakeController=true;feedShakeSources(229.0);assert(mergedSwing.mIsTriggerSwing);
 shakeController=false;feedShakeSources(229.1);assert(!mergedSwing.mIsTriggerSwing);
 std::puts("PASS keyboard/controller shake source union through original WPad swing: overlap and handoff do not retrigger; release rearms");
 delete[] read.mStatusArray;
 std::puts("PASS native KPAD to original WPadButton press/hold/repeat/release; disconnect release, repeated disconnect and reconnect verified");
}


















