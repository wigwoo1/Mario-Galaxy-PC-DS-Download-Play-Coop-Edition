#pragma once
#include "scenario_assembly.hpp"
#include "stage_scene.hpp"
#include "scenario_gravity.hpp"
#include "coin_pose.hpp"
inline StageScene loadScenarioZoneScene(WiiAssets& assets,const ScenarioAssembly& assembly,const std::string& zone){
    StageScene scene;SceneModelCache cache(assets,scene);auto gravityField=loadScenarioZoneGravity(assets,assembly,zone);
    for(const auto& record:assembly.placements){if((!zone.empty()&&record.zone!=zone)||std::filesystem::path(record.placement.table).filename()!="objinfo")continue;
        auto name=record.placement.name;auto archivePath=resolveObjectArchive(assets,name);
        if(!archivePath){scene.skipped.push_back(name+": no primary/direct archive");continue;}
        const auto& prototype=cache.load(*archivePath);if(!prototype.skip.empty()){scene.skipped.push_back(name+": "+prototype.skip);continue;}auto vertices=prototype.triangles;
        assetCheck(scene.triangles.size()+vertices.size()<=10000000,"Scenario zone preview budget exceeded");
        if(name=="Coin"||name=="PurpleCoin"){CollisionPoint position{record.world[3],record.world[7],record.world[11]},gravity{};if(auto selected=selectGravity(gravityField.sources,position))gravity=selected->direction;auto pose=nativeCoinPose(record.world,gravity,record.placement.arguments[4]>0);scene.coins.push_back({scene.triangles.size(),pose,prototype.triangles,scenarioActorKey(record)});}
        for(auto& vertex:vertices){vertex.position=transformPoint(record.world,vertex.position);vertex.normal=skinNormal(record.world,vertex.normal,vertex.normal);}
        scene.triangles.insert(scene.triangles.end(),vertices.begin(),vertices.end());scene.loaded.push_back(name+":"+prototype.path+" layer="+std::filesystem::path(record.placement.table).parent_path().filename().string());
    }assetCheck(!scene.triangles.empty(),"No supported selected scenario-zone models");return scene;
}




