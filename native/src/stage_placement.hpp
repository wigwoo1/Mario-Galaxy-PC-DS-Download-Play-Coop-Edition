#pragma once
#include "stage_data.hpp"
#include "skinning.hpp"
inline constexpr std::array<const char*,5> stageSwitchFields{"SW_A","SW_B","SW_APPEAR","SW_DEAD","SW_SLEEP"};
struct StagePlacement {
    std::string table,name;size_t row;std::array<int32_t,5> switches{-1,-1,-1,-1,-1};std::array<int32_t,8> arguments{-1,-1,-1,-1,-1,-1,-1,-1};
    std::array<float,3> position{0,0,0},rotationDegrees{0,0,0},scale{1,1,1};
};
inline std::vector<StagePlacement> decodePlacements(const std::string& path,const StageTable& table){
    std::vector<StagePlacement> result;int name=table.find("name");if(name<0)return result;
    const char* positions[]={"pos_x","pos_y","pos_z"};const char* rotations[]={"dir_x","dir_y","dir_z"};const char* scales[]={"scale_x","scale_y","scale_z"};
    for(size_t row=0;row<table.rows;++row){StagePlacement placement;placement.table=path;placement.row=row;auto value=table.read(row,size_t(name));
        assetCheck(std::holds_alternative<std::string>(value),"Placement name is not a string");placement.name=std::get<std::string>(value);
        auto read=[&](const char* key,float fallback){int field=table.find(key);if(field<0)return fallback;auto v=table.read(row,size_t(field));assetCheck(std::holds_alternative<float>(v),"Placement transform is not a float");return std::get<float>(v);};
        for(size_t i=0;i<3;++i){placement.position[i]=read(positions[i],0);placement.rotationDegrees[i]=read(rotations[i],0);placement.scale[i]=read(scales[i],1);}for(size_t i=0;i<placement.arguments.size();++i){auto key="Obj_arg"+std::to_string(i);int field=table.find(key);if(field>=0){auto argument=table.read(row,size_t(field));assetCheck(std::holds_alternative<uint32_t>(argument),"Placement argument is not an integer");placement.arguments[i]=int32_t(std::get<uint32_t>(argument));}}for(size_t i=0;i<stageSwitchFields.size();++i){int field=table.find(stageSwitchFields[i]);if(field>=0){auto value=table.read(row,size_t(field));assetCheck(std::holds_alternative<uint32_t>(value),"Placement switch is not an integer");placement.switches[i]=int32_t(std::get<uint32_t>(value));}}result.push_back(placement);
    }return result;
}
inline AffineMatrix placementMatrix(const StagePlacement& p){
    constexpr double radians=3.14159265358979323846/180;
    for(size_t i=0;i<3;++i)assetCheck(std::isfinite(p.position[i])&&std::isfinite(p.rotationDegrees[i])&&std::isfinite(p.scale[i]),"Invalid placement transform");
    float sx=float(std::sin(p.rotationDegrees[0]*radians)),cx=float(std::cos(p.rotationDegrees[0]*radians));
    float sy=float(std::sin(p.rotationDegrees[1]*radians)),cy=float(std::cos(p.rotationDegrees[1]*radians));
    float sz=float(std::sin(p.rotationDegrees[2]*radians)),cz=float(std::cos(p.rotationDegrees[2]*radians));
    AffineMatrix m{cz*cy,sx*cz*sy-cx*sz,cx*cz*sy+sx*sz,p.position[0],sz*cy,sx*sz*sy+cx*cz,cx*sz*sy-sx*cz,p.position[1],-sy,cy*sx,cy*cx,p.position[2]};
    columnScale(m,p.scale);return m;
}
inline void placementTests(){
    StagePlacement placement;placement.position={10,20,30};placement.scale={2,3,4};placement.rotationDegrees={0,0,90};
    auto point=transformPoint(placementMatrix(placement),{1,1,1});assetCheck(std::abs(point[0]-7)<.0001&&std::abs(point[1]-22)<.0001&&std::abs(point[2]-34)<.0001,"Placement SRT order test");
    StagePlacement zone;zone.position={100,0,0};zone.rotationDegrees={0,0,90};auto world=concatenate(placementMatrix(zone),placementMatrix(placement));
    auto a=transformPoint(world,{1,1,1}),b=transformPoint(placementMatrix(zone),point);for(size_t i=0;i<3;++i)assetCheck(std::abs(a[i]-b[i])<.0001,"Nested zone placement composition test");
}


