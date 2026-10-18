#include <revolution/gx/GXLighting.h>
#include <Native/GxCommandState.hpp>
#include <Game/Map/LightFunction.hpp>
#include <Game/Map/LightPointCtrl.hpp>
#include <revolution/gd/GDBase.h>
#include <cassert>
#include <cstdio>
// Only the camera accessor is a fixture dependency; light loading bodies are original.
namespace {TPos3f view,inverseView;}
namespace MR {const TPos3f& getCameraViewMtx(){return view;}const TPos3f& getCameraInvViewMtx(){return inverseView;}}
int main(){
 view.identity();view.mMtx[0][3]=10;view.mMtx[1][3]=20;view.mMtx[2][3]=30;
 inverseView.identity();inverseView.mMtx[0][3]=-10;inverseView.mMtx[1][3]=-20;inverseView.mMtx[2][3]=-30;
 u8 bytes[600]{};GDLObj writer;GDInitGDLObj(&writer,bytes,sizeof(bytes));GDSetCurrent(&writer);
 LightFunction::initLightRegisterAll();assert(GDGetCurrOffset()==552);Native::GxCommandState state;state.consume(bytes,GDGetCurrOffset());
 for(unsigned i=0;i<8;++i){auto light=state.light(i);assert((light.color==std::array<unsigned,4>{255,255,255,255}));assert((light.angular==std::array<float,3>{0,0,1}));assert((light.distance==std::array<float,3>{0,0,1}));}
 ActorLightInfo info{};info.mInfo0={{11,23,37,51},{1,2,3},true};info.mInfo1={{67,79,89,101},{4,5,6},false};info.mAlpha2=113;info.mColor={127,139,151,163};
 GDInitGDLObj(&writer,bytes,sizeof(bytes));LightFunction::loadActorLightInfo(&info);assert(GDGetCurrOffset()==216);state.consume(bytes,GDGetCurrOffset());
 assert((state.light(0).position==std::array<float,3>{1,2,3}));assert((state.light(1).position==std::array<float,3>{14,25,36}));
 assert((state.light(0).color==std::array<unsigned,4>{11,23,37,51}));assert((state.light(2).color==std::array<unsigned,4>{0,0,0,113}));assert((state.lightingColor(0,true)==std::array<unsigned,4>{127,139,151,163}));
 PointLightInfo point{{2,3,4},{17,31,47,61},10,.5f,GX_DA_GENTLE};GDInitGDLObj(&writer,bytes,sizeof(bytes));LightFunction::loadPointLightInfo(&point);state.consume(bytes,GDGetCurrOffset());
 auto light=state.light(4);assert((light.position==std::array<float,3>{12,23,34}));assert((light.color==std::array<unsigned,4>{17,31,47,61}));assert(light.angular[0]==1&&std::fabs(light.distance[1]-.1f)<.00001f);
 LightInfoCoin coin{};coin.base={{23,41,59,77},{7,8,9},true};coin._14={83,101,127,149};coin._18=1;
 GDInitGDLObj(&writer,bytes,sizeof(bytes));LightFunction::loadLightInfoCoin(&coin);assert(GDGetCurrOffset()==138);state.consume(bytes,GDGetCurrOffset());
 assert((state.light(0).position==std::array<float,3>{7,8,9}));assert((state.light(0).color==std::array<unsigned,4>{23,41,59,77}));
 auto specular=state.light(3);assert((specular.color==std::array<unsigned,4>{83,101,127,149}));assert((specular.angular==std::array<float,3>{0,0,1}));assert((specular.distance==std::array<float,3>{.5f,0,.5f}));
 GDInitGDLObj(&writer,bytes,sizeof(bytes));LightFunction::loadAllLightWhite();assert(GDGetCurrOffset()==552);state.consume(bytes,GDGetCurrOffset());
 for(unsigned slot=0;slot<8;++slot){auto white=state.light(slot);assert((white.color==std::array<unsigned,4>{255,255,255,255}));assert((white.position==std::array<float,3>{0,0,0}));}
 ActorLightInfo from{},to{},blended{};from.mInfo0.mPos={2,3,4};to.mInfo0.mPos={8,9,10};from.mAlpha2=1;to.mAlpha2=10;
 for(bool sourceCamera:{false,true})for(bool outputCamera:{false,true}){
  from.mInfo0.mIsFollowCamera=sourceCamera;to.mInfo0.mIsFollowCamera=outputCamera;blended.mInfo0.mIsFollowCamera=outputCamera;
  LightFunction::blendActorLightInfo(&blended,from,to,.5f);
  float expected=sourceCamera==outputCamera?5.f:outputCamera?10.f:0.f;assert(blended.mInfo0.mPos.x==expected&&blended.mAlpha2==5);
 }
 LightInfo location{{0,0,0,0},{2,3,4},true};TVec3f world;LightFunction::calcLightWorldPos(&world,location);assert(world.x==-8&&world.y==-17&&world.z==-26);
 location.mIsFollowCamera=false;LightFunction::calcLightWorldPos(&world,location);assert(world.x==2&&world.y==3&&world.z==4);
 GDSetCurrent(nullptr);std::puts("PASS original actor/point light loaders: 8-slot initialization, camera/world transforms, alpha light, ambient color, coin specular light and white-light reset");
}



