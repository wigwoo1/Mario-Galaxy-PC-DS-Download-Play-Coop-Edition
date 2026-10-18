#pragma once
#include "stage_placement.hpp"
#include "mesh_render.hpp"
#include "collision_query.hpp"
#include "collision_resource.hpp"
struct StageCollision {
    struct RuntimeTerrain {std::vector<uint8_t> kcl,metadata;AffineMatrix matrix;std::string name;};
    std::vector<RuntimeTerrain> runtimeTerrain;
    CollisionGeometry geometry;
    std::vector<std::string> instances,skipped;
};
inline StageCollision loadCommonCollision(WiiAssets& assets,const std::string& stagePath,bool captureRuntime=false){
    Rarc stage(assets.readFile(stagePath));StageCollision result;
    for(const auto& resource:stage.files){if(resource.path!="/jmp/placement/common/objinfo")continue;
        auto bytes=stage.readFile(resource.path);StageTable table{BinaryView(bytes)};table.validate();
        for(const auto& placement:decodePlacements(resource.path,table)){
            std::string path="/ObjectData/"+placement.name+".arc";bool found=false;for(const auto& file:assets.files)if(file.path==path){found=true;break;}
            if(!found){result.skipped.push_back(placement.name+": no direct archive");continue;}
            Rarc archive(assets.readFile(path));std::vector<std::string> candidates;
            for(const auto& file:archive.files)if(std::filesystem::path(file.path).extension()==".kcl")candidates.push_back(file.path);
            auto selected=selectMainCollision(placement.name,candidates);if(!selected){result.skipped.push_back(placement.name+": collision resource count="+std::to_string(candidates.size()));continue;}
            auto collisionBytes=archive.readFile(*selected);auto geometry=decodeCollision(BinaryView(collisionBytes));auto matrix=placementMatrix(placement);
            for(auto& triangle:geometry.triangles){for(auto& point:triangle.points)point=transformPoint(matrix,point);triangle.normal=skinNormal(matrix,triangle.normal,triangle.normal);triangle.instance=result.instances.size();}
            assetCheck(result.geometry.triangles.size()+geometry.triangles.size()<=1000000,"Zone collision preview budget exceeded");
            result.geometry.triangles.insert(result.geometry.triangles.end(),geometry.triangles.begin(),geometry.triangles.end());result.geometry.prisms+=geometry.prisms;result.geometry.degenerate+=geometry.degenerate;
            if(captureRuntime){auto metadataPath=std::filesystem::path(*selected).replace_extension(".pa").generic_string();bool exists=false;for(const auto& file:archive.files)if(file.path==metadataPath){exists=true;break;}assetCheck(exists,"Selected runtime collision has no matching metadata");result.runtimeTerrain.push_back({std::move(collisionBytes),archive.readFile(metadataPath),matrix,placement.name});}
            result.instances.push_back(placement.name+":"+*selected);
        }
    }
    assetCheck(!result.geometry.triangles.empty(),"No supported common-layer collision");return result;
}
inline void placedCollisionTests(){
    CollisionGeometry geometry;geometry.triangles={{{CollisionPoint{0,0,0},CollisionPoint{2,0,0},CollisionPoint{0,2,0}},{0,0,1},7,1,3}};
    StagePlacement placement;placement.position={10,20,30};placement.rotationDegrees={0,90,0};placement.scale={2,3,4};auto matrix=placementMatrix(placement);
    for(auto& triangle:geometry.triangles){for(auto& point:triangle.points)point=transformPoint(matrix,point);triangle.normal=skinNormal(matrix,triangle.normal,triangle.normal);}
    auto hit=traceCollision(geometry,{12,21,29},{8,21,29});assetCheck(hit&&std::abs(hit->fraction-.5)<.00001&&hit->instance==3&&hit->attribute==7&&std::abs(hit->normal[0]-1)<.0001,"Placed collision coordinate/metadata test");
}
