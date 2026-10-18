#pragma once
#include "scenario_assembly.hpp"
#include "stage_collision.hpp"
inline StageCollision loadScenarioZoneCollision(WiiAssets& assets,const ScenarioAssembly& assembly,const std::string& zone){
    StageCollision result;
    for(const auto& record:assembly.placements){if((!zone.empty()&&record.zone!=zone)||std::filesystem::path(record.placement.table).filename()!="objinfo")continue;
        auto name=record.placement.name;std::string path="/ObjectData/"+name+".arc";bool found=false;for(const auto& file:assets.files)if(file.path==path){found=true;break;}
        if(!found){result.skipped.push_back(name+": no direct archive");continue;}Rarc archive(assets.readFile(path));std::vector<std::string> files;
        for(const auto& file:archive.files)if(std::filesystem::path(file.path).extension()==".kcl")files.push_back(file.path);
        auto selected=selectMainCollision(name,files);if(!selected){result.skipped.push_back(name+": collision count="+std::to_string(files.size()));continue;}
        auto bytes=archive.readFile(*selected);auto geometry=decodeCollision(BinaryView(bytes));
        for(auto& triangle:geometry.triangles){for(auto& point:triangle.points)point=transformPoint(record.world,point);triangle.normal=skinNormal(record.world,triangle.normal,triangle.normal);triangle.instance=result.instances.size();}
        assetCheck(result.geometry.triangles.size()+geometry.triangles.size()<=1000000,"Scenario collision preview budget exceeded");result.geometry.triangles.insert(result.geometry.triangles.end(),geometry.triangles.begin(),geometry.triangles.end());result.geometry.prisms+=geometry.prisms;result.geometry.degenerate+=geometry.degenerate;
        result.instances.push_back(name+":"+*selected+" layer="+std::filesystem::path(record.placement.table).parent_path().filename().string());
    }assetCheck(!result.geometry.triangles.empty(),"No supported selected scenario-zone collision");return result;
}

