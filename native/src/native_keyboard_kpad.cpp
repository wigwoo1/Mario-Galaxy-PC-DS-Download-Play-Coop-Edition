#include <Native/PointerMotion.hpp>
#include <Native/WpadHost.hpp>
#include <mutex>
#include <cmath>
#include <chrono>
#include <Native/GameInput.hpp>
#include <Native/KeyboardKpad.hpp>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <xinput.h>
namespace {
std::mutex gameWindowMutex;void* gameWindow=nullptr;int shakeVirtualKey='Q';WORD shakeButtonMask=XINPUT_GAMEPAD_X;
int pointerViewportX=0,pointerViewportY=0,pointerViewportWidth=0,pointerViewportHeight=0;
const auto gameInputEpoch=std::chrono::steady_clock::now();
using XInputPoll=DWORD(WINAPI*)(DWORD,XINPUT_STATE*);
XInputPoll xinputPoll(){static HMODULE module=[](){HMODULE loaded=LoadLibraryW(L"xinput1_4.dll");return loaded?loaded:LoadLibraryW(L"xinput9_1_0.dll");}();static auto poll=module?reinterpret_cast<XInputPoll>(GetProcAddress(module,"XInputGetState")):nullptr;return poll;}
}
void nativeBindGameInputWindow(void* window){std::lock_guard<std::mutex> lock(gameWindowMutex);gameWindow=window;}
void nativeSetGameInputShakeBindings(int virtualKey,unsigned short buttonMask){
 std::lock_guard<std::mutex> lock(gameWindowMutex);shakeVirtualKey=(virtualKey>=0&&virtualKey<=255)?virtualKey:0;shakeButtonMask=static_cast<WORD>(buttonMask);
}
bool nativeSetGamePointerViewport(int x,int y,int width,int height){
 if(x<0||y<0||width<=0||height<=0||static_cast<long long>(x)+width>32767||static_cast<long long>(y)+height>32767)return false;
 std::lock_guard<std::mutex> lock(gameWindowMutex);pointerViewportX=x;pointerViewportY=y;pointerViewportWidth=width;pointerViewportHeight=height;return true;
}
void nativePumpGameInputFrame(){
 void* window;int shakeKey;WORD shakeButton;{std::lock_guard<std::mutex> lock(gameWindowMutex);window=gameWindow;shakeKey=shakeVirtualKey;shakeButton=shakeButtonMask;}
 if(!window){nativeSetHostWpadConnected(false);return;}
 const auto now=std::chrono::steady_clock::now();const double seconds=std::chrono::duration<double>(now-gameInputEpoch).count();
 nativePollWindowsKpad(window,seconds,shakeKey,shakeButton);
}
bool nativePollWindowsKpad(void* window,double seconds,int shakeKey,unsigned short shakeButtonMask){
 if(!window || !IsWindow(static_cast<HWND>(window))){nativeSetHostWpadConnected(false);return false;}
 static std::mutex pollingMutex;std::lock_guard<std::mutex> lock(pollingMutex);
 static NativePointerMotion motion;static void* priorWindow=nullptr;static bool shakeHeld=false;
 NativeKeyboardState input{};HWND hwnd=static_cast<HWND>(window);int viewportX=0,viewportY=0,viewportWidth=0,viewportHeight=0;
 {std::lock_guard<std::mutex> lock(gameWindowMutex);viewportX=pointerViewportX;viewportY=pointerViewportY;viewportWidth=pointerViewportWidth;viewportHeight=pointerViewportHeight;}
 nativeSetHostWpadConnected(true);
 if(GetForegroundWindow()==hwnd){
  auto down=[](int key){return (GetAsyncKeyState(key)&0x8000)!=0;};
  input.left=down('A');input.right=down('D');input.forward=down('W');input.back=down('S');
  input.jump=down(VK_SPACE);input.starbit=down(VK_SHIFT);input.crouch=down(VK_CONTROL);input.camera=down('C');input.pause=down(VK_ESCAPE);
  bool shakeDown=shakeKey&&down(shakeKey);
  XINPUT_STATE pad{};const auto poll=xinputPoll();const bool controllerConnected=poll&&poll(0,&pad)==ERROR_SUCCESS;if(controllerConnected)shakeDown=shakeDown||((pad.Gamepad.wButtons&shakeButtonMask)!=0);
  input.shake=nativeShakeImpulse(shakeDown,shakeHeld);
  input.pointerTrigger=down(VK_LBUTTON);
  RECT bounds{};POINT pointer{};
  if(GetClientRect(hwnd,&bounds)&&GetCursorPos(&pointer)&&ScreenToClient(hwnd,&pointer)&&bounds.right>0&&bounds.bottom>0){
   const int clientWidth=bounds.right,clientHeight=bounds.bottom;
   if(viewportWidth<=0||viewportHeight<=0)nativeMapClientPointer(input,pointer.x,pointer.y,clientWidth,clientHeight);
   else nativeMapClientPointer(input,pointer.x,pointer.y,clientWidth,clientHeight,viewportX,viewportY,viewportWidth,viewportHeight);
  }
  const bool controllerActivity=controllerConnected&&(pad.Gamepad.wButtons||pad.Gamepad.bLeftTrigger||pad.Gamepad.bRightTrigger||std::abs(int(pad.Gamepad.sThumbLX))>8192||std::abs(int(pad.Gamepad.sThumbLY))>8192||std::abs(int(pad.Gamepad.sThumbRX))>8192||std::abs(int(pad.Gamepad.sThumbRY))>8192);
  const bool keyboardActivity=input.left||input.right||input.forward||input.back||input.jump||input.starbit||input.crouch||input.camera||input.pause||shakeDown||input.pointerTrigger||controllerActivity;
  const bool pointerActivity=input.pointerValid&&(!motion.valid||priorWindow!=window||std::abs(input.pointerX-motion.x)>0.0001f||std::abs(input.pointerY-motion.y)>0.0001f);
  if(keyboardActivity||pointerActivity)nativeNotifyVideoActivity();
 }else shakeHeld=false;
 auto sample=nativeMapKeyboard(input);auto candidate=priorWindow==window?motion:NativePointerMotion{};
 candidate.prepare(sample);const bool submitted=nativeSubmitKpadState(0,sample,seconds);
 if(submitted){candidate.commit(sample);motion=candidate;priorWindow=window;}
 nativeDispatchWpadCallbacks();return submitted;
}



