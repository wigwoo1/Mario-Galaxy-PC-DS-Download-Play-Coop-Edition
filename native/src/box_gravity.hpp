#pragma once
#include "point_gravity.hpp"
struct NativeBoxGravity {
    NativePointGravity parameters;AffineMatrix box=identityMatrix();CollisionPoint up{0,1,0};float baseDistance=2000;int distanceAxis=-1;
    std::optional<double> distance(const CollisionPoint& position)const{
        double selected=0;
        for(size_t axis=0;axis<3;++axis){double dot=0,extent=0;for(size_t k=0;k<3;++k){assetCheck(std::isfinite(position[k]),"Invalid box gravity position");double component=box[k*4+axis];dot+=(double(position[k])-box[k*4+3])*component;extent+=component*component;}
            assetCheck(extent>0,"Zero box gravity extent");if(dot < -extent||dot>extent)return {};if(int(axis)==distanceAxis)selected=std::abs(dot)/std::sqrt(extent);}
        return baseDistance+selected;
    }
    std::optional<CollisionPoint> direction(const CollisionPoint& position)const{if(!distance(position))return {};auto value=up;for(float& v:value)v*=parameters.inverse?1:-1;return value;}
};
inline NativeBoxGravity decodeBoxGravity(const StagePlacement& placement,const StageTable& table){
    NativeBoxGravity gravity;auto defaults=placement;defaults.scale[0]=0;gravity.parameters=decodePointGravity(defaults,table);
    auto rotation=placement;rotation.scale={1,1,1};auto matrix=placementMatrix(rotation);gravity.up={matrix[1],matrix[5],matrix[9]};
    for(size_t k=0;k<3;++k){assetCheck(placement.scale[k]>0,"Non-positive box gravity scale");matrix[k*4+3]=placement.position[k]+gravity.up[k]*placement.scale[1]*500;}
    columnScale(matrix,{placement.scale[0]*500,placement.scale[1]*500,placement.scale[2]*500});gravity.box=matrix;
    auto argument=[&](const char* name){int field=table.find(name);if(field<0)return int32_t(-1);auto value=table.read(placement.row,size_t(field));assetCheck(std::holds_alternative<uint32_t>(value),"Box gravity argument mismatch");return int32_t(std::get<uint32_t>(value));};
    int32_t base=argument("Obj_arg0"),axis=argument("Obj_arg1");gravity.baseDistance=base<0?2000:float(base);gravity.distanceAxis=axis>=0&&axis<=2?axis:-1;return gravity;
}
inline void boxGravityTests(){
    NativeBoxGravity gravity;gravity.box={2,0,0,0,0,3,0,3,0,0,4,0};
    assetCheck(gravity.direction({2,0,4})==std::optional<CollisionPoint>({0,-1,0})&&!gravity.direction({2.01f,0,0}),"Box gravity inclusive bounds test");
    gravity.distanceAxis=1;assetCheck(gravity.distance({0,5,0})&&std::abs(*gravity.distance({0,5,0})-2002)<1e-8,"Box gravity distance axis test");
    gravity.parameters.inverse=true;assetCheck((*gravity.direction({0,3,0}))[1]==1,"Box inverse gravity test");
}
