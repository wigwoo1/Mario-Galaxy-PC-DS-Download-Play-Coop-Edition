#pragma once
#include "collision_index.hpp"
struct SphereSeparation {CollisionPoint position;size_t iterations=0;double remaining=0;bool resolved=false;};
inline SphereSeparation separateSphere(const CollisionIndex& index,CollisionPoint position,float radius){
    constexpr double tolerance=.001;SphereSeparation result{position};
    for(size_t iteration=0;iteration<24;++iteration){auto contacts=index.sphere(result.position,radius);const SphereContact* deepest=nullptr;
        for(const auto& contact:contacts)if(!deepest||contact.penetration>deepest->penetration)deepest=&contact;
        result.remaining=deepest?deepest->penetration:0;if(result.remaining<=tolerance){result.resolved=true;return result;}
        for(size_t k=0;k<3;++k)result.position[k]=float(double(result.position[k])+deepest->normal[k]*(deepest->penetration+tolerance));++result.iterations;
    }
    result.remaining=0;for(const auto& contact:index.sphere(result.position,radius))result.remaining=std::max(result.remaining,contact.penetration);result.resolved=result.remaining<=tolerance;return result;
}
inline void collisionResponseTests(){
    CollisionGeometry floor;floor.triangles={{{CollisionPoint{-100,-100,0},CollisionPoint{100,-100,0},CollisionPoint{0,100,0}},{0,0,1},0,1}};
    CollisionIndex index(floor);auto result=separateSphere(index,{0,0,.25f},1);assetCheck(result.resolved&&result.position[2]>=1&&result.position[2]<1.01&&result.iterations==1,"Sphere face separation test");
    auto stable=separateSphere(index,result.position,1);assetCheck(stable.resolved&&stable.iterations==0&&stable.position==result.position,"Sphere separation stability test");
    auto clear=separateSphere(index,{0,0,4},1);assetCheck(clear.resolved&&clear.position==CollisionPoint{0,0,4},"Clear sphere moved unexpectedly");
    floor.triangles.push_back({{CollisionPoint{0,-100,-100},CollisionPoint{0,100,-100},CollisionPoint{0,0,100}},{1,0,0},0,2});
    CollisionIndex corner(floor);auto cornerResult=separateSphere(corner,{.25f,0,.25f},1);assetCheck(cornerResult.resolved&&cornerResult.position[0]>=1&&cornerResult.position[2]>=1,"Sphere corner separation test");
}
inline std::pair<size_t,size_t> verifySphereSeparation(const CollisionGeometry& geometry){
    CollisionIndex index(geometry);size_t resolved=0,unresolved=0,stride=std::max<size_t>(1,geometry.triangles.size()/32);
    for(size_t i=0;i<geometry.triangles.size();i+=stride){const auto& triangle=geometry.triangles[i];CollisionPoint center{};double length=std::sqrt(double(collisionDot(triangle.normal,triangle.normal)));
        for(size_t k=0;k<3;++k)center[k]=float((double(triangle.points[0][k])+triangle.points[1][k]+triangle.points[2][k])/3+5*triangle.normal[k]/length);
        auto result=separateSphere(index,center,10);double remaining=0;for(const auto& contact:sphereContacts(geometry,result.position,10))remaining=std::max(remaining,contact.penetration);
        assetCheck(result.iterations<=24&&std::abs(remaining-result.remaining)<1e-8,"Sphere response residual mismatch");
        if(result.resolved){assetCheck(remaining<=.001,"Resolved sphere remains penetrating");++resolved;}else ++unresolved;
    }return {resolved,unresolved};
}
