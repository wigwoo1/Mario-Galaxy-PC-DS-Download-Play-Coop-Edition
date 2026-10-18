#pragma once
#include "collision_index.hpp"
struct SphereSweep {double fraction=0;CollisionPoint position{},normal{};size_t iterations=0;bool blocked=false,complete=false;};
inline SphereSweep sweepSphere(const CollisionIndex& index,const CollisionPoint& start,const CollisionPoint& end,float radius){
    assetCheck(std::isfinite(radius)&&radius>0,"Invalid sweep radius");CollisionBox box;ContactVector direction{};double length2=0;
    for(size_t k=0;k<3;++k){assetCheck(std::isfinite(start[k])&&std::isfinite(end[k]),"Invalid sweep endpoints");direction[k]=double(end[k])-start[k];length2+=direction[k]*direction[k];box.low[k]=std::min(double(start[k]),double(end[k]))-radius-.001;box.high[k]=std::max(double(start[k]),double(end[k]))+radius+.001;}
    auto candidates=index.candidates(box);
    // A triangle cannot be reached if the whole segment stays farther than
    // the radius from its supporting plane. This also avoids tiny conservative
    // steps during near-surface parallel travel.
    candidates.triangles.erase(std::remove_if(candidates.triangles.begin(),candidates.triangles.end(),[&](const CollisionTriangle& triangle){
        CollisionPoint edge1{},edge2{};for(size_t k=0;k<3;++k){edge1[k]=triangle.points[1][k]-triangle.points[0][k];edge2[k]=triangle.points[2][k]-triangle.points[0][k];}
        auto normal=collisionCross(edge1,edge2);double n=std::sqrt(double(collisionDot(normal,normal)));if(n<1e-12)return false;
        double a=0,b=0;for(size_t k=0;k<3;++k){a+=(double(start[k])-triangle.points[0][k])*normal[k]/n;b+=(double(end[k])-triangle.points[0][k])*normal[k]/n;}
        return (a>radius+.002&&b>radius+.002)||(a<-radius-.002&&b<-radius-.002);
    }),candidates.triangles.end());
    SphereSweep result;result.position=start;double length=std::sqrt(length2);
    if(candidates.triangles.empty()){result.position=end;result.fraction=1;result.complete=true;return result;}
    constexpr double skin=.001;
    for(size_t iteration=0;iteration<128;++iteration){result.iterations=iteration+1;double distance=INFINITY;ContactVector nearest{};const CollisionTriangle* triangle=nullptr;
        for(const auto& candidate:candidates.triangles){auto point=closestTrianglePoint(candidate,result.position);ContactVector delta{};for(size_t k=0;k<3;++k)delta[k]=double(result.position[k])-point[k];double value=std::sqrt(contactDot(delta,delta));if(value<distance){distance=value;nearest=point;triangle=&candidate;}}
        if(distance<=radius+skin){result.blocked=true;result.complete=true;double normalLength=std::sqrt(double(collisionDot(triangle->normal,triangle->normal)));assetCheck(normalLength>0,"Invalid sweep face normal");
            for(size_t k=0;k<3;++k)result.normal[k]=float(distance>1e-12?(result.position[k]-nearest[k])/distance:triangle->normal[k]/normalLength);return result;}
        if(length<1e-12){result.fraction=1;result.position=end;result.complete=true;return result;}
        // Distance to a closed triangle is 1-Lipschitz: advancing less than the
        // current surface gap cannot cross it, regardless of face/edge region.
        double step=.9*(distance-radius)/length;
        if(result.fraction+step>=1){result.position=end;result.fraction=1;}else{result.fraction+=step;for(size_t k=0;k<3;++k)result.position[k]=float(double(start[k])+result.fraction*direction[k]);}
        if(result.fraction==1){result.complete=true;return result;}
    }
    // Exhaustion is a partial advance, never reported as unobstructed travel.
    return result;
}
inline void collisionSweepTests(){
    CollisionGeometry geometry;geometry.triangles={{{CollisionPoint{-100,-100,0},CollisionPoint{100,-100,0},CollisionPoint{0,100,0}},{0,0,1},0,1}};CollisionIndex index(geometry);
    auto face=sweepSphere(index,{0,0,100},{0,0,-100},1);assetCheck(face.complete&&face.blocked&&face.position[2]>=.999&&face.position[2]<1.01&&std::abs(face.fraction-.495)<.0001,"Fast sphere thin-plane sweep failed");
    auto reverse=sweepSphere(index,{0,0,-100},{0,0,100},1);assetCheck(reverse.blocked&&reverse.position[2]<=-.999,"Reverse sphere sweep failed");
    auto clear=sweepSphere(index,{0,0,3},{10,0,3},1);assetCheck(clear.complete&&!clear.blocked&&clear.fraction==1&&clear.position==CollisionPoint{10,0,3},"Clear sphere sweep failed");
    auto stationary=sweepSphere(index,{0,0,3},{0,0,3},1);assetCheck(stationary.complete&&!stationary.blocked,"Stationary sphere sweep failed");
    auto overlapping=sweepSphere(index,{0,0,.5f},{0,0,3},1);assetCheck(overlapping.blocked&&overlapping.fraction==0,"Initial overlap must be handled before sweep");
}
inline size_t verifySphereSweeps(const CollisionGeometry& geometry){
    CollisionIndex index(geometry);size_t tested=0,stride=std::max<size_t>(1,geometry.triangles.size()/32);
    for(size_t i=0;i<geometry.triangles.size();i+=stride){const auto& triangle=geometry.triangles[i];CollisionPoint start{},end{};double length=std::sqrt(double(collisionDot(triangle.normal,triangle.normal)));
        for(size_t k=0;k<3;++k){double center=(double(triangle.points[0][k])+triangle.points[1][k]+triangle.points[2][k])/3;start[k]=float(center+30*triangle.normal[k]/length);end[k]=float(center-30*triangle.normal[k]/length);}
        auto result=sweepSphere(index,start,end,10);assetCheck(result.complete&&result.blocked&&result.fraction<.5&&result.iterations<=128,"Retail sphere sweep missed target surface");++tested;
    }return tested;
}
