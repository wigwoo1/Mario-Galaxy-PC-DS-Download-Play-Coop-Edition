#pragma once
#include "collision_response.hpp"
#include "collision_sweep.hpp"
struct SphereMovement {CollisionPoint position;size_t contacts=0;bool complete=false;std::vector<CollisionPoint> normals;};
inline SphereMovement slideSphere(const CollisionIndex& index,const CollisionPoint& start,const CollisionPoint& displacement,float radius){
    for(float value:displacement)assetCheck(std::isfinite(value),"Invalid sphere movement");auto separated=separateSphere(index,start,radius);SphereMovement result{separated.position};if(!separated.resolved)return result;
    CollisionPoint remaining=displacement;
    for(size_t iteration=0;iteration<8;++iteration){double length2=0;for(float value:remaining)length2+=double(value)*value;if(length2<1e-12){result.complete=true;break;}
        CollisionPoint end{};for(size_t k=0;k<3;++k)end[k]=result.position[k]+remaining[k];auto sweep=sweepSphere(index,result.position,end,radius);result.position=sweep.position;
        if(!sweep.complete)return result;if(!sweep.blocked){result.complete=true;break;}++result.contacts;result.normals.push_back(sweep.normal);
        for(float& value:remaining)value=float(value*(1-sweep.fraction));double into=collisionDot(remaining,sweep.normal);
        if(into<0)for(size_t k=0;k<3;++k)remaining[k]=float(remaining[k]-into*sweep.normal[k]);
        // Leave enough separation for the next tangent sweep's plane rejection.
        for(size_t k=0;k<3;++k)result.position[k]+=sweep.normal[k]*.004f;
    }
    auto final=separateSphere(index,result.position,radius);result.position=final.position;if(!final.resolved)result.complete=false;return result;
}
inline void collisionSlideTests(){
    CollisionGeometry geometry;geometry.triangles={{{CollisionPoint{-100,-100,0},CollisionPoint{100,-100,0},CollisionPoint{0,100,0}},{0,0,1},0,1}};CollisionIndex index(geometry);
    auto slide=slideSphere(index,{0,0,3},{10,0,-6},1);assetCheck(slide.complete&&slide.contacts==1&&std::abs(slide.position[0]-10)<.01&&slide.position[2]>=1&&slide.position[2]<1.02,"Floor sliding test");
    auto parallel=sweepSphere(index,{0,0,1.005f},{50,0,1.005f},1);assetCheck(parallel.complete&&!parallel.blocked&&parallel.position[0]==50,"Near-floor tangent sweep stalled");
    auto clear=slideSphere(index,{0,0,3},{1,2,0},1);assetCheck(clear.complete&&clear.contacts==0&&clear.position==CollisionPoint{1,2,3},"Clear sphere movement test");
    geometry.triangles.push_back({{CollisionPoint{0,-100,-100},CollisionPoint{0,100,-100},CollisionPoint{0,0,100}},{1,0,0},0,2});CollisionIndex corner(geometry);
    auto blocked=slideSphere(corner,{3,0,3},{-6,10,-6},1);assetCheck(blocked.complete&&blocked.position[0]>=1&&blocked.position[2]>=1&&std::abs(blocked.position[1]-10)<.05,"Corner sliding test");
}
inline std::pair<size_t,size_t> verifySphereMovement(const CollisionGeometry& geometry){
    CollisionIndex index(geometry);size_t complete=0,partial=0,stride=std::max<size_t>(1,geometry.triangles.size()/32);
    for(size_t i=0;i<geometry.triangles.size();i+=stride){const auto& triangle=geometry.triangles[i];CollisionPoint start{},movement{};double length=std::sqrt(double(collisionDot(triangle.normal,triangle.normal)));
        for(size_t k=0;k<3;++k){start[k]=float((double(triangle.points[0][k])+triangle.points[1][k]+triangle.points[2][k])/3+30*triangle.normal[k]/length);movement[k]=float(-40*triangle.normal[k]/length);}
        auto result=slideSphere(index,start,movement,10);double penetration=0;for(const auto& contact:sphereContacts(geometry,result.position,10))penetration=std::max(penetration,contact.penetration);
        if(result.complete){assetCheck(penetration<=.001,"Completed sphere movement remains penetrating");++complete;}else ++partial;
    }return {complete,partial};
}
