#pragma once
#include "graphics.hpp"
using CollisionPoint=std::array<float,3>;
inline CollisionPoint collisionCross(const CollisionPoint& a,const CollisionPoint& b){return {a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]};}
inline float collisionDot(const CollisionPoint& a,const CollisionPoint& b){return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];}
struct CollisionTriangle {std::array<CollisionPoint,3> points;CollisionPoint normal;uint16_t attribute;size_t prism;size_t instance=0;};
struct CollisionGeometry {std::vector<CollisionTriangle> triangles;size_t prisms=0,degenerate=0;float thickness=0;};
inline CollisionGeometry decodeCollision(BinaryView view){
    extent(0,56,view.size());size_t positions=view.u32(0),normals=view.u32(4),prisms=view.u32(8),octree=view.u32(12);
    assetCheck(positions>=56&&positions<=normals&&normals<=prisms&&prisms+16<=octree&&octree<view.size(),"Invalid KCL section offsets");
    // Vector arrays may have alignment padding before the next section.
    assetCheck((octree-prisms)%16==0,"Misaligned KCL prism array");
    CollisionGeometry geometry;geometry.thickness=view.f32(16);assetCheck(geometry.thickness>=0,"Negative KCL thickness");
    // The stored prism pointer is biased back one reserved record; its first
    // 16 bytes may overlap the end of the normal array on disk.
    size_t positionCount=(normals-positions)/12,normalCount=(prisms+16-normals)/12;geometry.prisms=(octree-prisms)/16-1;
    auto vector=[&](size_t base,size_t index,size_t count){assetCheck(index<count,"Invalid KCL vector index");size_t p=base+index*12;return CollisionPoint{view.f32(p),view.f32(p+4),view.f32(p+8)};};
    // Prism zero is the reserved sentinel. Reconstruct vertices as KCollisionServer::getPos.
    for(size_t i=1;i<=geometry.prisms;++i){size_t p=prisms+i*16;float height=view.f32(p);auto origin=vector(positions,view.u16(p+4),positionCount);
        auto face=vector(normals,view.u16(p+6),normalCount),edge0=vector(normals,view.u16(p+8),normalCount),edge1=vector(normals,view.u16(p+10),normalCount),edge2=vector(normals,view.u16(p+12),normalCount);
        auto side1=collisionCross(edge1,face),side2=collisionCross(face,edge0);float d1=collisionDot(side1,edge2),d2=collisionDot(side2,edge2);
        if(height<=0||std::abs(d1)<1e-12f||std::abs(d2)<1e-12f){++geometry.degenerate;continue;}
        CollisionTriangle triangle{{origin,origin,origin},face,view.u16(p+14),i};
        for(size_t axis=0;axis<3;++axis){triangle.points[1][axis]+=side1[axis]*(height/d1);triangle.points[2][axis]+=side2[axis]*(height/d2);
            assetCheck(std::isfinite(triangle.points[1][axis])&&std::isfinite(triangle.points[2][axis]),"Non-finite KCL triangle");}
        geometry.triangles.push_back(triangle);
    }return geometry;
}
