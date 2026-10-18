#pragma once
#include "scenario_assembly.hpp"
#include "gravity_field.hpp"
inline CollisionPoint rotateGravityAxis(const AffineMatrix& matrix,const CollisionPoint& axis){CollisionPoint result{};for(size_t r=0;r<3;++r)for(size_t c=0;c<3;++c)result[r]+=matrix[r*4+c]*axis[c];return result;}
inline ZoneGravityField loadScenarioZoneGravity(WiiAssets& assets,const ScenarioAssembly& assembly,const std::string& zone){
    ZoneGravityField result;
    for(const auto& record:assembly.placements){if((!zone.empty()&&record.zone!=zone)||std::filesystem::path(record.placement.table).filename()!="planetobjinfo")continue;
        for(size_t column=0;column<3;++column){double length=0;for(size_t row=0;row<3;++row)length+=double(record.parent[row*4+column])*record.parent[row*4+column];assetCheck(std::abs(length-1)<.001,"Scaled parent-zone gravity not supported");}
        Rarc archive(assets.readFile("/StageData/"+record.zone+".arc"));auto bytes=archive.readFile(record.placement.table);StageTable table{BinaryView(bytes)};GravitySource source;auto name=record.placement.name;
        if(name=="GlobalPointGravity"){auto point=decodePointGravity(record.placement,table);point.center=transformPoint(record.parent,point.center);source.shape=point;}
        else if(name=="GlobalDiskGravity"){auto disk=decodeDiskGravity(record.placement,table);disk.parameters.center=transformPoint(record.parent,disk.parameters.center);disk.normal=rotateGravityAxis(record.parent,disk.normal);disk.sectorAxis=rotateGravityAxis(record.parent,disk.sectorAxis);source.shape=disk;}
        else if(name=="GlobalPlaneGravityInBox"){auto box=decodeBoxGravity(record.placement,table);box.parameters.center=transformPoint(record.parent,box.parameters.center);box.box=concatenate(record.parent,box.box);box.up=rotateGravityAxis(record.parent,box.up);source.shape=box;}
        else if(name=="GlobalSegmentGravity"){auto segment=decodeSegmentGravity(record.placement,table);segment.parameters.center=transformPoint(record.parent,segment.parameters.center);segment.end=transformPoint(record.parent,segment.end);segment.axis=rotateGravityAxis(record.parent,segment.axis);segment.sectorAxis=rotateGravityAxis(record.parent,segment.sectorAxis);source.shape=segment;}
        else{++result.unsupported;continue;}source.name=name;int field=table.find("Priority");if(field>=0){auto value=table.read(record.placement.row,size_t(field));assetCheck(std::holds_alternative<uint32_t>(value),"Scenario gravity priority type mismatch");int32_t priority=int32_t(std::get<uint32_t>(value));if(priority>=0)source.priority=priority;}result.sources.push_back(source);
    }return result;
}
inline void scenarioGravityTests(){
    StagePlacement parent;parent.position={100,20,30};parent.rotationDegrees={0,0,90};auto matrix=placementMatrix(parent);auto up=rotateGravityAxis(matrix,{0,1,0});assetCheck(std::abs(up[0]+1)<.0001&&std::abs(up[1])<.0001,"Parent gravity rotation test");
    NativePointGravity source;source.center=transformPoint(matrix,{0,0,0});auto direction=source.direction(transformPoint(matrix,{1,0,0}));assetCheck(direction&&std::abs((*direction)[1]+1)<.0001,"Parent point gravity translation test");
}

