#pragma once
#include "point_gravity.hpp"
struct NativeDiskGravity {
    NativePointGravity parameters;CollisionPoint normal{0,1,0},sectorAxis{1,0,0};float radius=500,sectorCos=-1;bool bothSides=true,edgeGravity=true;
    std::optional<CollisionPoint> direction(const CollisionPoint& position)const{
        CollisionPoint relative{},radial{};for(size_t k=0;k<3;++k){assetCheck(std::isfinite(position[k]),"Invalid disk gravity position");relative[k]=position[k]-parameters.center[k];}
        float height=collisionDot(relative,normal);if(!bothSides&&height<0)return {};for(size_t k=0;k<3;++k)radial[k]=relative[k]-height*normal[k];float distanceAxis=std::sqrt(collisionDot(radial,radial));
        if(distanceAxis>.001)for(float& value:radial)value/=distanceAxis;else radial={0,0,0};if(sectorCos>-1&&collisionDot(radial,sectorAxis)<sectorCos)return {};
        CollisionPoint gravity{};float distance=std::abs(height);
        if(distanceAxis<=radius){for(size_t k=0;k<3;++k)gravity[k]=height>=0?-normal[k]:normal[k];}
        else{if(!edgeGravity)return {};for(size_t k=0;k<3;++k)gravity[k]=parameters.center[k]+radial[k]*radius-position[k];distance=std::sqrt(collisionDot(gravity,gravity));if(distance>.001)for(float& value:gravity)value/=distance;else gravity={0,0,0};}
        if(parameters.range>=0&&distance>=parameters.range+parameters.distant)return {};if(parameters.inverse)for(float& value:gravity)value=-value;return gravity;
    }
};
inline NativeDiskGravity decodeDiskGravity(const StagePlacement& placement,const StageTable& table){
    NativeDiskGravity disk;auto defaults=placement;defaults.scale[0]=0;disk.parameters=decodePointGravity(defaults,table);disk.radius=500*std::max({placement.scale[0],placement.scale[1],placement.scale[2]});
    assetCheck(disk.radius>0,"Invalid disk gravity radius");auto rotation=placement;rotation.position={0,0,0};rotation.scale={1,1,1};auto matrix=placementMatrix(rotation);
    disk.normal={matrix[1],matrix[5],matrix[9]};CollisionPoint side{matrix[0],matrix[4],matrix[8]};
    auto argument=[&](const char* name){int field=table.find(name);if(field<0)return int32_t(-1);auto value=table.read(placement.row,size_t(field));assetCheck(std::holds_alternative<uint32_t>(value),"Disk gravity argument type mismatch");return int32_t(std::get<uint32_t>(value));};
    disk.bothSides=argument("Obj_arg0")!=0;disk.edgeGravity=argument("Obj_arg1")!=0;int32_t angle=argument("Obj_arg2");float degrees=angle<0?360.0f:float(angle);
    float half=degrees*.5f*3.14159265358979323846f/180;disk.sectorCos=std::cos(half);auto cross=collisionCross(disk.normal,side);
    for(size_t k=0;k<3;++k)disk.sectorAxis[k]=side[k]*std::cos(half)+cross[k]*std::sin(half);return disk;
}
inline void diskGravityTests(){
    NativeDiskGravity disk;disk.radius=2;auto above=disk.direction({1,3,0}),below=disk.direction({1,-3,0});assetCheck(above&&(*above)[1]==-1&&below&&(*below)[1]==1,"Disk face direction test");
    auto edge=disk.direction({3,1,0});assetCheck(edge&&std::abs((*edge)[0]+.70710678f)<.0001&&std::abs((*edge)[1]+.70710678f)<.0001,"Disk edge direction test");
    disk.bothSides=false;assetCheck(!disk.direction({0,-1,0}),"One-sided disk test");disk.edgeGravity=false;assetCheck(!disk.direction({3,1,0}),"Disabled disk edge test");
    disk.parameters.range=3;assetCheck(!disk.direction({0,3,0}),"Disk range boundary test");disk.parameters.range=-1;disk.sectorCos=0;disk.sectorAxis={1,0,0};assetCheck(!disk.direction({-1,1,0})&&disk.direction({1,1,0}),"Disk sector test");
}
