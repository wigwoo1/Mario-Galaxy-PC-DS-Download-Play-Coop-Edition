#pragma once
#include <windows.h>
#include <xinput.h>
#include "assets.hpp"
inline std::array<float,2> nativeStickVector(short x,short y){float fx=x,fy=y;float length=std::sqrt(fx*fx+fy*fy);constexpr float deadzone=XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;if(length<=deadzone)return {0,0};float magnitude=std::min(1.f,(length-deadzone)/(32767-deadzone));return {fx/length*magnitude,fy/length*magnitude};}
inline std::array<float,2> nativeKeyboardVector(int horizontal,int vertical,bool slow){assetCheck(horizontal>=-1&&horizontal<=1&&vertical>=-1&&vertical<=1,"Invalid keyboard movement axis");float x=float(horizontal),y=float(vertical),length=std::sqrt(x*x+y*y);if(length==0)return {0,0};float scale=(slow?.25f:1.f)/length;return {x*scale,y*scale};}
class NativeGamepad {
 HMODULE library=nullptr;using Poll=DWORD(WINAPI*)(DWORD,XINPUT_STATE*);Poll poll=nullptr;bool heldJump=false;
public:
 struct Input{std::array<float,2> stick{};bool jump=false;};
 NativeGamepad(){library=LoadLibraryW(L"xinput1_4.dll");if(!library)library=LoadLibraryW(L"xinput9_1_0.dll");if(library)poll=reinterpret_cast<Poll>(GetProcAddress(library,"XInputGetState"));}
 ~NativeGamepad(){if(library)FreeLibrary(library);}
 NativeGamepad(const NativeGamepad&)=delete;NativeGamepad& operator=(const NativeGamepad&)=delete;
 Input read(bool active){Input result;XINPUT_STATE state{};if(!active||!poll||poll(0,&state)!=ERROR_SUCCESS){heldJump=false;return result;}result.stick=nativeStickVector(state.Gamepad.sThumbLX,state.Gamepad.sThumbLY);bool down=(state.Gamepad.wButtons&XINPUT_GAMEPAD_A)!=0;result.jump=down&&!heldJump;heldJump=down;return result;}
};
inline void nativeStickTests(){for(int x:{-1,0,1})for(int y:{-1,0,1})for(bool slow:{false,true}){auto keyboard=nativeKeyboardVector(x,y,slow);float magnitude=std::sqrt(keyboard[0]*keyboard[0]+keyboard[1]*keyboard[1]);assetCheck(std::abs(magnitude-((x||y)?(slow?.25f:1.f):0.f))<1e-6f,"Keyboard cardinal/diagonal speed mismatch");}assetCheck(nativeStickVector(0,0)==std::array<float,2>{0,0}&&nativeStickVector(7849,0)[0]==0,"Controller deadzone mismatch");auto full=nativeStickVector(32767,0);assetCheck(full[0]==1&&full[1]==0,"Controller cardinal normalization mismatch");auto diagonal=nativeStickVector(-32768,32767);float length=std::sqrt(diagonal[0]*diagonal[0]+diagonal[1]*diagonal[1]);assetCheck(std::abs(length-1)<1e-6f&&diagonal[0]<0&&diagonal[1]>0,"Controller diagonal normalization mismatch");auto mid=nativeStickVector(20000,0);assetCheck(mid[0]>0&&mid[0]<1,"Controller intermediate magnitude lost");}

