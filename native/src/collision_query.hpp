#pragma once
#include "collision_geometry.hpp"
#include <optional>
struct CollisionHit {double fraction;CollisionPoint point,normal;uint16_t attribute;size_t prism;size_t instance=0;};
inline std::optional<CollisionHit> traceCollision(const CollisionGeometry& geometry,const CollisionPoint& start,const CollisionPoint& end){
    std::array<double,3> direction{};double length2=0;
    for(size_t i=0;i<3;++i){assetCheck(std::isfinite(start[i])&&std::isfinite(end[i]),"Invalid collision segment");direction[i]=double(end[i])-start[i];length2+=direction[i]*direction[i];}
    if(length2<1e-20)return {};
    auto cross=[](const std::array<double,3>& a,const std::array<double,3>& b){return std::array<double,3>{a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]};};
    auto dot=[](const std::array<double,3>& a,const std::array<double,3>& b){return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];};
    std::optional<CollisionHit> nearest;
    for(const auto& triangle:geometry.triangles){
        std::array<double,3> edge1{},edge2{},offset{};
        for(size_t i=0;i<3;++i){edge1[i]=double(triangle.points[1][i])-triangle.points[0][i];edge2[i]=double(triangle.points[2][i])-triangle.points[0][i];offset[i]=double(start[i])-triangle.points[0][i];}
        auto p=cross(direction,edge2);double determinant=dot(edge1,p);
        double scale=std::sqrt(dot(edge1,edge1)*dot(edge2,edge2)*length2);if(scale==0||std::abs(determinant)<=scale*1e-12)continue;
        double u=dot(offset,p)/determinant;if(u<-1e-8||u>1+1e-8)continue;
        auto q=cross(offset,edge1);double v=dot(direction,q)/determinant;if(v<-1e-8||u+v>1+1e-8)continue;
        double t=dot(edge2,q)/determinant;if(t<-1e-8||t>1+1e-8)continue;t=std::max(0.0,std::min(1.0,t));
        if(nearest&&t>=nearest->fraction)continue;
        CollisionHit hit{t,{},triangle.normal,triangle.attribute,triangle.prism,triangle.instance};for(size_t i=0;i<3;++i)hit.point[i]=float(start[i]+direction[i]*t);nearest=hit;
    }return nearest;
}
inline void collisionQueryTests(){
    CollisionGeometry geometry;geometry.triangles={{{CollisionPoint{0,0,0},CollisionPoint{2,0,0},CollisionPoint{0,2,0}},{0,0,1},7,1},
        {{CollisionPoint{0,0,-2},CollisionPoint{2,0,-2},CollisionPoint{0,2,-2}},{0,0,1},9,2}};
    auto hit=traceCollision(geometry,{.5f,.5f,1},{.5f,.5f,-3});assetCheck(hit&&std::abs(hit->fraction-.25)<1e-8&&hit->prism==1&&hit->attribute==7,"Nearest collision segment test");
    auto reverse=traceCollision(geometry,{.5f,.5f,-3},{.5f,.5f,1});assetCheck(reverse&&reverse->prism==2,"Two-sided collision segment test");
    assetCheck(traceCollision(geometry,{1,1,1},{1,1,-1}).has_value(),"Shared-edge collision test");
    assetCheck(!traceCollision(geometry,{2,2,1},{2,2,-1})&&!traceCollision(geometry,{0,0,1},{1,0,1})&&!traceCollision(geometry,{0,0,0},{0,0,0}),"Collision miss/parallel/zero segment test");
}
inline size_t verifyCollisionQueries(const CollisionGeometry& geometry){
    assetCheck(!geometry.triangles.empty(),"Empty collision geometry");size_t tested=0,stride=std::max<size_t>(1,geometry.triangles.size()/32);
    for(size_t index=0;index<geometry.triangles.size();index+=stride){const auto& triangle=geometry.triangles[index];CollisionPoint centroid{},start{},end{};
        float length=std::sqrt(collisionDot(triangle.normal,triangle.normal));assetCheck(length>0,"Zero collision face normal");
        for(size_t i=0;i<3;++i){centroid[i]=(triangle.points[0][i]+triangle.points[1][i]+triangle.points[2][i])/3;start[i]=centroid[i]+10*triangle.normal[i]/length;end[i]=centroid[i]-10*triangle.normal[i]/length;}
        CollisionGeometry single;single.triangles={triangle};auto isolated=traceCollision(single,start,end);auto scene=traceCollision(geometry,start,end);
        assetCheck(isolated&&std::abs(isolated->fraction-.5)<.001&&scene&&scene->fraction<=isolated->fraction+1e-8,"Retail collision segment verification failed");++tested;
    }return tested;
}
