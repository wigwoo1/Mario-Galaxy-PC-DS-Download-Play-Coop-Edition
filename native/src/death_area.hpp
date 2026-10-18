#pragma once
#include "area_volume.hpp"
#include "stage_switch_state.hpp"
#include "Game/AreaObj/DeathArea.hpp"
struct NativeFatalAreaEvent{TVec3f position;bool requested=false;};
class NativeFatalAreaScope{public:explicit NativeFatalAreaScope(NativeFatalAreaEvent&);~NativeFatalAreaScope();NativeFatalAreaScope(const NativeFatalAreaScope&)=delete;NativeFatalAreaScope& operator=(const NativeFatalAreaScope&)=delete;};
class NativeDeathArea:public DeathArea{
    AffineMatrix world;StageSwitchCtrl control{JMapInfoIter{}};NativeAreaVolumeBinding volume;
public:
    NativeDeathArea(int form,const AffineMatrix& matrix,NativeStageSwitchState& state,std::string zone,std::array<int32_t,5> switches):DeathArea(form,"native death area"),world(matrix),volume(this,[this](const TVec3f& p){return nativeDeathVolumeContains(mFormType,world,p);}){
        for(auto id:switches)if(id>=0)state.registerSwitch(zone,id);std::array<std::function<bool()>,3> readers;for(size_t i=0;i<3;++i)readers[i]=[&state,zone,switches,i](){return switches[i]>=0&&state.get(zone,switches[i]);};bindNativeStageSwitchCtrl(&control,std::move(readers),{switches[0]>=0,switches[1]>=0,switches[2]>=0,switches[3]>=0});mSwitchCtrl=&control;
    }
    ~NativeDeathArea()override{unbindNativeStageSwitchCtrl(&control);mSwitchCtrl=nullptr;}
    // Original fatal predicate runs after player physics through evaluate(), not during adapter scene tick.
    void movement()override{}
    bool evaluate(const TVec3f& position){NativeFatalAreaEvent event{position};NativeFatalAreaScope scope(event);DeathArea::movement();return event.requested;}
    std::array<float,3> interior()const{return mFormType==2?std::array<float,3>{world[3],world[7],world[11]}:transformPoint(world,{0,500,0});}
};
inline void deathAreaTests(){
    NativeScene scene;NativeStageSwitchState state;auto& cube=scene.create<NativeDeathArea>(1,identityMatrix(),state,"test",std::array<int32_t,5>{1,-1,-1,-1,-1});
    assetCheck(!cube.evaluate({0,500,0}),"Original death area ignored off switch A");state.set("test",1,true);assetCheck(cube.evaluate({0,500,0})&&!cube.evaluate({500,500,0}),"Original death area cube trigger/boundary mismatch");cube.mObjArg0=1;assetCheck(!cube.evaluate({0,500,0}),"AnyNoPlayer death area killed player");cube.mObjArg0=-1;cube.invalidate();assetCheck(!cube.evaluate({0,500,0}),"Invalid death area killed player");cube.validate();cube.sleep();assetCheck(!cube.evaluate({0,500,0}),"Sleeping death area killed player");cube.awake();assetCheck(cube.evaluate({0,500,0}),"Awakened death area did not request fatal event");
    auto& sphere=scene.create<NativeDeathArea>(2,identityMatrix(),state,"test",std::array<int32_t,5>{-1,-1,-1,-1,-1});assetCheck(sphere.evaluate({499,0,0})&&!sphere.evaluate({500,0,0}),"Original sphere radius boundary mismatch");
    auto& cylinder=scene.create<NativeDeathArea>(3,identityMatrix(),state,"test",std::array<int32_t,5>{-1,-1,-1,-1,-1});assetCheck(cylinder.evaluate({0,0,0})&&cylinder.evaluate({0,1000,0})&&!cylinder.evaluate({500,500,0})&&!cylinder.evaluate({0,1001,0}),"Original cylinder axial/radial boundary mismatch");
}
