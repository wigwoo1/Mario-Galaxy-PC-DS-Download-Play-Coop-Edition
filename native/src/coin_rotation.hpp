#pragma once
#include "assets.hpp"
#include "native_scene.hpp"
#include "stage_scene.hpp"
// Native matrix/scene adaptation of CoinRotater constructor and movement policy.
class NativeCoinRotater:public NameObj {
    static AffineMatrix rotation(float angle){StagePlacement pose;pose.rotationDegrees={0,angle,0};pose.scale={1,1,1};return placementMatrix(pose);}
public:
    uint64_t ticks=0;float normalAngle=0,waterAngle=0,fastAngle=0;
    AffineMatrix normal=rotation(0),water=rotation(0),fast=rotation(0);
    NativeCoinRotater():NameObj("CoinRotater"){}
    void movement()override{++ticks;normalAngle=std::fmod(normalAngle+8.0f,360.0f);waterAngle=std::fmod(waterAngle+4.0f,360.0f);fastAngle=std::fmod(fastAngle+16.0f,360.0f);normal=rotation(normalAngle);water=rotation(waterAngle);fast=rotation(fastAngle);}
};
inline void coinRotationTests(){StageScene geometry;RenderVertex vertex{};vertex.position={1,0,0};vertex.normal={0,1,0};vertex.texture=3;geometry.triangles={vertex};StagePlacement placement;placement.position={10,20,30};placement.scale={1,1,1};geometry.coins.push_back({0,placementMatrix(placement),{vertex}});NativeScene visualScene;auto& visual=visualScene.create<NativeCoinRotater>();visualScene.tick();auto rotated=stageSceneWithCoinRotation(geometry,visual.normal);assetCheck(rotated.size()==1&&rotated[0].texture==3&&std::abs(rotated[0].position[0]-(10+std::cos(8.0f*3.14159265358979323846f/180)))<.0001f&&rotated[0].position[1]==20,"Coin retained geometry transform failed");assetCheck(geometry.triangles[0].position[0]==1,"Coin render mutated source geometry");assetCheck(stageSceneWithCoinRotation(geometry,visual.normal,[](const std::string&){return false;}).empty(),"Hidden coin geometry remained visible");geometry.triangles.push_back(vertex);auto filtered=stageSceneWithCoinRotation(geometry,visual.normal,[](const std::string&){return false;});assetCheck(filtered.size()==1&&filtered[0].position==vertex.position,"Coin visibility removed unrelated geometry");NativeScene scene;auto& actor=scene.create<NativeCoinRotater>();scene.tick();assetCheck(actor.normalAngle==8&&actor.waterAngle==4&&actor.fastAngle==16,"Coin rotation first tick mismatch");for(int i=1;i<90;++i)scene.tick();assetCheck(actor.normalAngle==0&&actor.waterAngle==0&&actor.fastAngle==0,"Coin rotation wrap mismatch");auto identity=actor.normal;assetCheck(identity[0]==1&&identity[5]==1&&identity[10]==1,"Coin rotation identity mismatch");actor.requestSuspend();scene.tick();assetCheck(actor.normalAngle==0,"Suspended coin rotation advanced");actor.requestResume();scene.tick();assetCheck(actor.normalAngle==8,"Coin rotation resume failed");auto point=transformPoint(actor.normal,{1,0,0});assetCheck(std::abs(point[0]-std::cos(8.0f*3.14159265358979323846f/180.0f))<0.0001f&&std::abs(point[1])<0.0001f&&std::abs(point[2]+std::sin(8.0f*3.14159265358979323846f/180.0f))<0.0001f,"Coin rotation matrix mismatch");}






