#include <Game/Map/LightPointCtrl.hpp>
#include <Game/Util/MathUtil.hpp>
#include <cassert>
#include <cstdio>
int main(){
 Vec a{1,5,-9},b{9,-3,7},out;MR::blendVec(&out,a,b,.25f);assert(out.x==3&&out.y==3&&out.z==-5);
 MR::blendVec(&a,a,b,.25f);assert(a.x==3&&a.y==3&&a.z==-5);
 GXColor ca{1,5,101,253},cb{10,14,110,2},color;MR::blendColor(&color,ca,cb,.25f);assert(color.r==3&&color.g==7&&color.b==103&&color.a==190);
 MR::blendColor(&cb,ca,cb,0);assert(cb.r==1&&cb.g==5&&cb.b==101&&cb.a==253);
 LightPointCtrl controller;assert(controller.mStep==-1&&controller.mBlendTime==30&&controller.mCurrentInfo->mRefDistance==15&&controller.mCurrentInfo->mColor.a==255);
 PointLightInfo from{{0,4,8},{0,20,40,60},10,.2f,GX_DA_STEEP},to{{8,12,16},{100,120,140,160},30,.6f,GX_DA_STEEP},result{};
 int currentToken=0,previousToken=0; // Identity tokens only; original blend method never dereferences actors.
 controller.mCurrentActor=reinterpret_cast<const LiveActor*>(&currentToken);controller.mPreviousActor=reinterpret_cast<const LiveActor*>(&previousToken);
 controller.blendPointLight(&result,from,to,.25f);assert(result.mPos.x==2&&result.mPos.y==6&&result.mPos.z==10);assert(std::fabs(result.mRefBrightness-.3f)<.00001f&&result.mRefDistance==15&&result.mColor.r==25&&result.mColor.a==85);
 controller.mPreviousActor=nullptr;controller.blendPointLight(&result,from,to,.25f);assert(result.mPos.x==8&&std::fabs(result.mRefBrightness-.8625f)<.00001f);
 controller.mPreviousActor=reinterpret_cast<const LiveActor*>(&previousToken);controller.mCurrentActor=nullptr;controller.blendPointLight(&result,from,to,.25f);assert(result.mPos.x==0&&std::fabs(result.mRefBrightness-.3875f)<.00001f);
 delete controller.mCurrentInfo;delete controller.mTargetInfo;delete controller.mPreviousInfo;
 std::puts("PASS original vector/color interpolation, in-place writes and point-light entering/leaving transitions");
}
