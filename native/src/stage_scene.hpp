#pragma once
#include "stage_placement.hpp"
#include "mesh_render.hpp"
#include "object_archive.hpp"
struct SceneCoinGeometry {size_t offset=0;AffineMatrix world;std::vector<RenderVertex> local;std::string actorKey;};
struct StageScene {
    std::vector<RenderVertex> triangles;
    std::vector<TextureImage> textures;
    std::vector<std::string> loaded,skipped;
    std::vector<SceneCoinGeometry> coins;
    size_t uniqueModels=0;
};
struct SceneModelPrototype {std::vector<RenderVertex> triangles;std::string path,skip;};
class SceneModelCache {
    WiiAssets& assets;StageScene& scene;std::map<std::string,SceneModelPrototype> prototypes;
public:
    SceneModelCache(WiiAssets& source,StageScene& target):assets(source),scene(target){}
    const SceneModelPrototype& load(const std::string& archivePath){
        auto found=prototypes.find(archivePath);if(found!=prototypes.end())return found->second;SceneModelPrototype prototype;Rarc archive(assets.readFile(archivePath));std::vector<std::string> models;
        for(const auto& file:archive.files){auto ext=std::filesystem::path(file.path).extension().string();if(ext==".bdl"||ext==".bmd")models.push_back(file.path);}
        if(models.size()!=1)prototype.skip="model count="+std::to_string(models.size());
        else{auto bytes=archive.readFile(models[0]);BinaryView view(bytes);auto model=decodeModel(view);auto rig=decodeRig(view);auto mesh=decodeMesh(view,model,rig);prototype.triangles=materialPoseTriangles(view,model,rig,mesh);prototype.path=models[0];size_t base=scene.textures.size();assetCheck(base+model.textures.size()<=INT32_MAX,"Scene texture index overflow");for(auto& vertex:prototype.triangles)if(vertex.texture>=0)vertex.texture+=int(base);scene.textures.insert(scene.textures.end(),model.textures.begin(),model.textures.end());++scene.uniqueModels;}
        return prototypes.emplace(archivePath,std::move(prototype)).first->second;
    }
};
inline StageScene loadCommonScene(WiiAssets& assets,const std::string& stagePath){
    Rarc stage(assets.readFile(stagePath));StageScene scene;SceneModelCache cache(assets,scene);
    for(const auto& resource:stage.files){
        if(resource.path!="/jmp/placement/common/objinfo")continue;
        auto bytes=stage.readFile(resource.path);StageTable table{BinaryView(bytes)};table.validate();
        for(const auto& placement:decodePlacements(resource.path,table)){
            auto archivePath=resolveObjectArchive(assets,placement.name);
            if(!archivePath){scene.skipped.push_back(placement.name+": no primary/direct archive");continue;}
            const auto& prototype=cache.load(*archivePath);if(!prototype.skip.empty()){scene.skipped.push_back(placement.name+": "+prototype.skip);continue;}
            auto triangles=prototype.triangles;auto matrix=placementMatrix(placement);
            for(auto& vertex:triangles){vertex.position=transformPoint(matrix,vertex.position);vertex.normal=skinNormal(matrix,vertex.normal,vertex.normal);}
            assetCheck(scene.triangles.size()+triangles.size()<=10000000,"Scene geometry exceeds preview budget");
            scene.triangles.insert(scene.triangles.end(),triangles.begin(),triangles.end());
            scene.loaded.push_back(placement.name+":"+prototype.path);
        }
    }
    assetCheck(!scene.triangles.empty(),"No supported common-layer scene models");return scene;
}
inline std::vector<RenderVertex> stageSceneWithCoinRotation(const StageScene& scene,const AffineMatrix& rotation,const std::function<bool(const std::string&)>& visible={}){auto vertices=scene.triangles;for(const auto& coin:scene.coins){assetCheck(coin.offset<=vertices.size()&&coin.local.size()<=vertices.size()-coin.offset,"Coin scene geometry range invalid");auto matrix=concatenate(coin.world,rotation);for(size_t i=0;i<coin.local.size();++i){auto vertex=coin.local[i];vertex.position=transformPoint(matrix,vertex.position);vertex.normal=skinNormal(matrix,vertex.normal,vertex.normal);vertices[coin.offset+i]=vertex;}}if(visible){std::vector<RenderVertex> filtered;size_t cursor=0;for(const auto& coin:scene.coins){assetCheck(coin.offset>=cursor,"Overlapping coin geometry");filtered.insert(filtered.end(),vertices.begin()+cursor,vertices.begin()+coin.offset);if(visible(coin.actorKey))filtered.insert(filtered.end(),vertices.begin()+coin.offset,vertices.begin()+coin.offset+coin.local.size());cursor=coin.offset+coin.local.size();}filtered.insert(filtered.end(),vertices.begin()+cursor,vertices.end());return filtered;}return vertices;}

