#include <Native/GxCommandState.hpp>
#include <revolution/gx/GXLighting.h>
#include <revolution/gd/GDBase.h>
#include <cassert>
#include <cstdio>
int main(){
 GXLightObj object{};GXInitLightColor(&object,{13,71,149,227});GXInitLightPos(&object,1,2,3);GXInitLightDir(&object,4,5,6);GXInitLightAttn(&object,1,2,3,4,5,6);
 u8 bytes[69]{};GDLObj writer;GDInitGDLObj(&writer,bytes,sizeof(bytes));GDSetCurrent(&writer);GXLoadLightObjImm(&object,GX_LIGHT4);assert(GDGetCurrOffset()==69);
 Native::GxCommandState state;state.consume(bytes,sizeof(bytes));auto light=state.light(4);
 assert((light.color==std::array<unsigned,4>{13,71,149,227}));assert((light.position==std::array<float,3>{1,2,3}));assert((light.direction==std::array<float,3>{-4,-5,-6}));assert((light.angular==std::array<float,3>{1,2,3}));assert((light.distance==std::array<float,3>{4,5,6}));
 GXInitLightSpot(&object,0,GX_SP_OFF);GXInitLightDistAttn(&object,10,.5f,GX_DA_GENTLE);GDInitGDLObj(&writer,bytes,sizeof(bytes));GXLoadLightObjImm(&object,GX_LIGHT0);state.consume(bytes,sizeof(bytes));light=state.light(0);assert(light.angular[0]==1&&light.angular[1]==0&&light.distance[0]==1&&std::fabs(light.distance[1]-.1f)<.00001f);
 GDInitGDLObj(&writer,bytes,68);bool rejected=false;try{GXLoadLightObjImm(&object,GX_LIGHT0);}catch(const std::exception&){rejected=true;}assert(rejected&&GDGetCurrOffset()==0);
 GDSetCurrent(nullptr);std::puts("PASS original GX light initialization -> native XF load -> decoded complete light, atomic capacity rejection");
}


