#pragma once
#include <Native/KpadInput.hpp>
#include <cmath>
struct NativeKeyboardState {
 bool left{},right{},forward{},back{},jump{},starbit{},pointerTrigger{},crouch{},camera{},pause{},shake{};
 bool pointerValid{};float pointerX{},pointerY{};
};
inline bool nativeShakeImpulse(bool held,bool& wasHeld){const bool pulse=held&&!wasHeld;wasHeld=held;return pulse;}
// Original KPAD screen coordinates increase downward on both axes.
inline bool nativeMapClientPointer(NativeKeyboardState& input,int x,int y,int width,int height){
 input.pointerValid=false;input.pointerX=input.pointerY=0;
 if(width<=0||height<=0)return false;
 input.pointerValid=x>=0&&y>=0&&x<width&&y<height;
 input.pointerX=2.0f*float(x)/float(width)-1.0f;
 input.pointerY=2.0f*float(y)/float(height)-1.0f;
 return true;
}
// Map host client coordinates against the actual game viewport. Bars outside
// the viewport intentionally produce an invalid Wii pointer sample.
inline bool nativeMapClientPointer(NativeKeyboardState& input,int x,int y,int clientWidth,int clientHeight,
                                   int viewportX,int viewportY,int viewportWidth,int viewportHeight){
 input.pointerValid=false;input.pointerX=input.pointerY=0;
 if(clientWidth<=0||clientHeight<=0||viewportX<0||viewportY<0||viewportWidth<=0||viewportHeight<=0)return false;
 const auto right=static_cast<long long>(viewportX)+viewportWidth;
 const auto bottom=static_cast<long long>(viewportY)+viewportHeight;
 if(right>clientWidth||bottom>clientHeight)return false;
 input.pointerValid=x>=viewportX&&y>=viewportY&&static_cast<long long>(x)<right&&static_cast<long long>(y)<bottom;
 if(input.pointerValid){
  input.pointerX=2.0f*float(x-viewportX)/float(viewportWidth)-1.0f;
  input.pointerY=2.0f*float(y-viewportY)/float(viewportHeight)-1.0f;
 }
 return true;
}
inline KPADStatus nativeMapKeyboard(const NativeKeyboardState& input){
 KPADStatus state{};state.dev_type=WPAD_DEV_FREESTYLE;state.data_format=WPAD_FMT_FREESTYLE_ACC_DPD;
 state.wpad_err=WPAD_ERR_NONE;state.acc.y=1;state.acc_value=1;state.horizon.x=1;state.dist=1;
 state.ex_status.fs.acc.y=1;state.ex_status.fs.acc_value=1;
 // The original WPadAcceleration provider consumes freestyle acceleration
 // history; a held bind supplies a short non-gravity impulse in that stream.
 if(input.shake)state.ex_status.fs.acc.x=2.0f;
 if(input.jump)state.hold|=WPAD_BUTTON_A;
 if(input.starbit||input.pointerTrigger)state.hold|=WPAD_BUTTON_B;
 if(input.crouch)state.hold|=WPAD_BUTTON_Z;
 if(input.camera)state.hold|=WPAD_BUTTON_C;
 if(input.pause)state.hold|=WPAD_BUTTON_PLUS;
 float x=float(input.right)-float(input.left),y=float(input.forward)-float(input.back);
 const float length=std::sqrt(x*x+y*y);if(length>1){x/=length;y/=length;}
 state.ex_status.fs.stick={x,y};
 state.dpd_valid_fg=input.pointerValid?2:0;
 if(input.pointerValid){state.pos.x=input.pointerX;state.pos.y=input.pointerY;}
 return state;
}
// Poll only when the supplied game window is foreground; inactive windows emit neutral input.
bool nativePollWindowsKpad(void* window,double seconds,int shakeVirtualKey='Q',unsigned short shakeButtonMask=0x0400);



