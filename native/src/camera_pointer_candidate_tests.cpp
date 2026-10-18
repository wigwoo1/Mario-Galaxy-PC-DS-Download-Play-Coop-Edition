#include <revolution/types.h>
#include "Game/Camera/CameraParamChunk.hpp"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <memory>
int main(){
 static_assert(sizeof(CameraGeneralParam::mNum1)==4);
 static_assert(sizeof(CameraGeneralParam::mAnimData)==8);
 CameraGeneralParam a,b;
 assert(a.mAnimData==nullptr && a.mNum1==0 && a.mNum2==0);
 auto data=std::make_unique<u8[]>(64);
 assert(reinterpret_cast<std::uintptr_t>(data.get())>UINT32_MAX);
 data[0]=0x73;data[63]=0x29;
 a.mAnimData=data.get();a.mNum1=-123456789;a.mNum2=456;
 a.mDist=123.5f;a.mAxis.set(3,4,5);a.mString.setCharPtr("animation");
 b=a;
 assert(b.mAnimData==data.get() && b.mAnimData[0]==0x73 && b.mAnimData[63]==0x29);
 assert(b.mNum1==-123456789 && b.mNum2==456 && b.mDist==123.5f);
 assert(b.mAxis.x==3 && b.mAxis.y==4 && b.mAxis.z==5);
 assert(b.mString.getCharPtr()==a.mString.getCharPtr());
 b=b;assert(b.mAnimData==data.get() && b.mNum1==-123456789);
 CameraGeneralParam empty;b=empty;
 assert(b.mAnimData==nullptr && b.mNum1==0 && b.mNum2==0 && b.mString.getCharPtr()==nullptr);
 assert(a.mAnimData==data.get() && data[63]==0x29);
 puts("PASS native camera pointer above 4GiB, numeric field preservation, assignment, self-copy and default reset");
}
