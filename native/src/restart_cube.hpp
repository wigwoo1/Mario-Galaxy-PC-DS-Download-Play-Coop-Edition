#pragma once
#include "stage_memory.hpp"
#include "stage_placement.hpp"
#include "native_scene.hpp"
#include "Game/AreaObj/RestartCube.hpp"
class NativeRestartScope{public:NativeRestartScope(GameDataTemporaryInGalaxy&,bool);~NativeRestartScope();NativeRestartScope(const NativeRestartScope&)=delete;NativeRestartScope& operator=(const NativeRestartScope&)=delete;};
// Original RestartCube class with native area geometry/lifetime integration.
class NativeRestartCube:public RestartCube{
    AffineMatrix world;
public:
    explicit NativeRestartCube(const AffineMatrix& matrix):RestartCube(1,"RestartCube"),world(matrix){}
    ~NativeRestartCube()override{delete mIdInfo;mIdInfo=nullptr;}
    bool isInVolume(const TVec3f& position)const override{
        if(!isValid())return false;
        std::array<double,3> a{world[0],world[4],world[8]},b{world[1],world[5],world[9]},c{world[2],world[6],world[10]},d{double(position.x)-world[3],double(position.y)-world[7],double(position.z)-world[11]};
        auto cross=[](const auto& x,const auto& y){return std::array<double,3>{x[1]*y[2]-x[2]*y[1],x[2]*y[0]-x[0]*y[2],x[0]*y[1]-x[1]*y[0]};};auto dot=[](const auto& x,const auto& y){return x[0]*y[0]+x[1]*y[1]+x[2]*y[2];};
        auto bc=cross(b,c);double det=dot(a,bc);assetCheck(std::isfinite(det)&&std::abs(det)>1e-12,"Singular restart area transform");
        double x=dot(d,bc)/det,y=dot(d,cross(c,a))/det,z=dot(d,cross(a,b))/det;
        return x>=-500&&x<500&&y>=0&&y<1000&&z>=-500&&z<500;
    }
    void updateRestart(NativeStageMemory& state,bool grounded){assetCheck(mIdInfo!=nullptr,"Uninitialized restart cube trigger");NativeRestartScope scope(state.result,grounded);updatePlayerRestartIdInfo();}
    std::array<float,3> interior()const{return transformPoint(world,{0,500,0});}
};
inline void restartCubeTests(){
    struct Table{u32 count,fields,offset,stride;JMapItem item;s32 start;} table{1,1,28,4,{0,0xffffffff,0,0,0},5};table.item.mHash=jmapHash("Obj_arg0");JMapInfo info;info.attach(&table);JMapInfoIter iter(&info,0);
    NativeScene scene;NativeStageMemory state;NativeRestartCube* cube=nullptr;{NativePlacedInfoScope context(iter,2);cube=&scene.place<NativeRestartCube>(iter,identityMatrix());}
    assetCheck(cube->isInVolume({-500,0,-500})&&!cube->isInVolume({500,0,0})&&!cube->isInVolume({0,1000,0})&&!cube->isInVolume({0,-1,0}),"Restart area half-open boundaries mismatch");
    cube->mObjArg3=0;cube->updateRestart(state,false);assetCheck(state.restartInfo()==JMapIdInfo(0,0),"Grounded restart trigger accepted airborne player");cube->updateRestart(state,true);assetCheck(state.restartInfo()==JMapIdInfo(5,2),"Original restart trigger did not set authored start/zone");
    state.resetRestart();cube->mObjArg3=-1;cube->updateRestart(state,false);assetCheck(state.restartInfo()==JMapIdInfo(5,2),"Unconditional restart trigger rejected airborne player");cube->invalidate();assetCheck(!cube->isInVolume({0,500,0}),"Invalid restart area accepted player");
    StagePlacement transformed;transformed.position={10,20,30};transformed.rotationDegrees={0,0,90};transformed.scale={2,3,4};auto& rotated=scene.create<NativeRestartCube>(placementMatrix(transformed));auto inside=rotated.interior();assetCheck(rotated.isInVolume({inside[0],inside[1],inside[2]}),"Transformed restart area rejected interior");
}

