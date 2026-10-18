#pragma once
#include "collision_geometry.hpp"
struct DiagnosticHeading {
    CollisionPoint forward{0,0,1},right{1,0,0};bool initialized=false;
    void update(const CollisionPoint& up,const CollisionPoint& input,const CollisionPoint& fallback){
        assetCheck(std::abs(collisionDot(up,up)-1)<.001,"Heading requires normalized gravity up");
        auto project=[&](CollisionPoint vector){float dot=collisionDot(vector,up);for(size_t k=0;k<3;++k)vector[k]-=dot*up[k];return vector;};
        auto direction=project(input);if(collisionDot(direction,direction)<1e-8)direction=project(initialized?forward:fallback);
        if(collisionDot(direction,direction)<1e-8&&initialized)direction=collisionCross(project(right),up);
        if(collisionDot(direction,direction)<1e-8)direction=project(fallback);
        float length=std::sqrt(collisionDot(direction,direction));assetCheck(length>1e-4&&std::isfinite(length),"No usable tangent heading");
        for(float& value:direction)value/=length;forward=direction;right=collisionCross(up,forward);initialized=true;
    }
};
inline void diagnosticHeadingTests(){
    StagePlacement start,parent;start.rotationDegrees={0,90,0};parent.rotationDegrees={0,0,90};auto world=concatenate(placementMatrix(parent),placementMatrix(start));CollisionPoint authored{world[2],world[6],world[10]},upVector{-1,0,0};DiagnosticHeading authoredHeading;authoredHeading.update(upVector,authored,{0,1,0});assetCheck(collisionDot(authoredHeading.forward,authored)>.999&&std::abs(collisionDot(authoredHeading.forward,upVector))<.0001,"Authored start/parent heading transform failed");
    DiagnosticHeading heading;heading.update({0,1,0},{1,0,1},{0,0,1});float root=std::sqrt(.5f);
    assetCheck(std::abs(heading.forward[0]-root)<.0001&&std::abs(heading.forward[2]-root)<.0001,"Diagonal heading normalization failed");
    auto initial=heading.forward;heading.update({0,1,0},{0,0,0},{0,0,-1});for(size_t k=0;k<3;++k)assetCheck(std::abs(heading.forward[k]-initial[k])<.0001,"Idle heading changed");
    heading.update({1,0,0},{0,0,0},{0,0,1});assetCheck(heading.forward==CollisionPoint{0,0,1}&&heading.right==CollisionPoint{0,-1,0},"Gravity heading projection failed");
    heading.update({0,0,1},{0,0,0},{0,1,0});assetCheck(heading.forward==CollisionPoint{-1,0,0},"Degenerate projected heading transport failed");
    DiagnosticHeading transported;transported.update({0,1,0},{0,0,1},{0,0,1});auto previousRight=transported.right;
    for(int step=1;step<=180;++step){float angle=float(step)*3.14159265358979323846f/180;CollisionPoint changingUp{std::sin(angle),std::cos(angle),0};auto axis=std::abs(changingUp[1])<.9f?CollisionPoint{0,1,0}:CollisionPoint{1,0,0};auto fallbackRight=collisionCross(axis,changingUp);float length=std::sqrt(collisionDot(fallbackRight,fallbackRight));for(float& value:fallbackRight)value/=length;auto fallback=collisionCross(changingUp,fallbackRight);transported.update(changingUp,{0,0,0},fallback);assetCheck(collisionDot(transported.right,previousRight)>.999f&&collisionDot(transported.forward,CollisionPoint{0,0,1})>.999f,"Smooth gravity movement basis flipped at reference-axis change");assetCheck(std::abs(collisionDot(transported.right,changingUp))<.0001f,"Transported movement basis left tangent plane");previousRight=transported.right;}
    DiagnosticHeading pole;pole.update({0,1,0},{0,0,1},{0,0,1});pole.update({0,0,1},{0,0,0},{0,1,0});assetCheck(collisionDot(pole.forward,CollisionPoint{0,-1,0})>.999f&&collisionDot(pole.right,CollisionPoint{1,0,0})>.999f,"Heading parallel to new gravity lost prior sideways direction");
    auto up=CollisionPoint{0,0,1};assetCheck(std::abs(collisionDot(heading.forward,up))<.0001&&std::abs(collisionDot(heading.right,up))<.0001&&std::abs(collisionDot(heading.right,heading.forward))<.0001,"Heading basis is not orthogonal");
}



