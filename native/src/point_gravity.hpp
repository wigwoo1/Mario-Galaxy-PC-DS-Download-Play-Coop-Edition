#pragma once
#include "stage_placement.hpp"
#include "collision_geometry.hpp"
#include <optional>
struct NativePointGravity {
    CollisionPoint center{};float range=-1,distant=0;bool inverse=false;
    std::optional<CollisionPoint> direction(const CollisionPoint& position)const{
        CollisionPoint delta{};double length2=0;for(size_t k=0;k<3;++k){assetCheck(std::isfinite(position[k]),"Invalid gravity position");delta[k]=center[k]-position[k];length2+=double(delta[k])*delta[k];}
        double length=std::sqrt(length2);if(range>=0&&length>=double(range)+distant)return {};
        if(length<.001)return CollisionPoint{0,0,0};for(float& value:delta)value=float(value/length*(inverse?-1:1));return delta;
    }
};
inline NativePointGravity decodePointGravity(const StagePlacement& placement,const StageTable& table){
    NativePointGravity gravity;gravity.center=placement.position;gravity.distant=500*placement.scale[0];
    auto optionalFloat=[&](const char* key,float fallback){int field=table.find(key);if(field<0)return fallback;auto value=table.read(placement.row,size_t(field));assetCheck(std::holds_alternative<float>(value),"Gravity float field mismatch");float result=std::get<float>(value);return result>=0?result:fallback;};
    gravity.range=optionalFloat("Range",gravity.range);gravity.distant=optionalFloat("Distant",gravity.distant);
    int inverse=table.find("Inverse");if(inverse>=0){auto value=table.read(placement.row,size_t(inverse));assetCheck(std::holds_alternative<uint32_t>(value),"Gravity inverse field mismatch");int32_t result=int32_t(std::get<uint32_t>(value));if(result>=0)gravity.inverse=result!=0;}
    return gravity;
}
inline void pointGravityTests(){
    NativePointGravity gravity;gravity.center={10,0,0};gravity.range=5;gravity.distant=2;
    auto inward=gravity.direction({13,0,0});assetCheck(inward&&(*inward)[0]==-1,"Point gravity direction test");
    assetCheck(!gravity.direction({17,0,0})&&gravity.direction({10,0,0})==std::optional<CollisionPoint>({0,0,0}),"Point gravity range/center test");
    gravity.inverse=true;assetCheck((*gravity.direction({13,0,0}))[0]==1,"Inverse point gravity test");gravity.range=-1;assetCheck(gravity.direction({1000,0,0}).has_value(),"Unlimited point gravity test");
}
