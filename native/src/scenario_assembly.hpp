#pragma once
#include "scenario_layers.hpp"
#include "stage_placement.hpp"
struct ScenarioPlacement {StagePlacement placement;AffineMatrix world;std::string zone;size_t zoneInstance;AffineMatrix parent=identityMatrix();std::optional<int32_t> startId;};
struct ScenarioAssembly {std::string galaxy;uint32_t scenarioNumber=0,powerStarCount=0;std::vector<ScenarioPlacement> placements;size_t zones=0,disabledTables=0;};
inline ScenarioAssembly assembleScenario(WiiAssets& assets,const std::string& galaxy,uint32_t number){
    auto layers=loadScenarioLayers(assets,galaxy,number);std::map<std::string,uint32_t> masks;for(auto zone:layers)assetCheck(masks.emplace(zone.name,zone.layers).second,"Duplicate scenario zone");
    ScenarioAssembly result;result.galaxy=galaxy;result.scenarioNumber=number;{Rarc metadata(assets.readFile("/StageData/"+galaxy+"/"+galaxy+"Scenario.arc"));auto bytes=metadata.readFile("/scenariodata.bcsv");StageTable table{BinaryView(bytes)};table.validate();int field=table.find("PowerStarId");if(field>=0)for(size_t row=0;row<table.rows;++row){auto value=table.read(row,size_t(field));assetCheck(std::holds_alternative<uint32_t>(value),"PowerStarId type mismatch");if(std::get<uint32_t>(value)!=0)++result.powerStarCount;}}std::vector<std::string> stack;
    std::function<void(const std::string&,const AffineMatrix&)> load=[&](const std::string& name,const AffineMatrix& parent){
        assetCheck(stack.size()<32&&result.zones<1024,"Scenario zone recursion exceeds budget");assetCheck(std::find(stack.begin(),stack.end(),name)==stack.end(),"Scenario zone cycle");auto mask=masks.find(name);assetCheck(mask!=masks.end(),"Referenced zone absent from scenario list");stack.push_back(name);size_t instance=result.zones++;
        Rarc archive(assets.readFile("/StageData/"+name+".arc"));
        for(const auto& resource:archive.files){auto path=std::filesystem::path(resource.path);std::string leaf=path.filename().string();
            if(resource.path.rfind("/jmp/placement/",0)!=0&&resource.path.rfind("/jmp/start/",0)!=0)continue;
            if(leaf!="objinfo"&&leaf!="planetobjinfo"&&leaf!="stageobjinfo"&&leaf!="startinfo"&&leaf!="areaobjinfo")continue;
            std::string layer=path.parent_path().filename().string();if(!scenarioLayerEnabled(layer,mask->second)){++result.disabledTables;continue;}
            auto bytes=archive.readFile(resource.path);StageTable table{BinaryView(bytes)};table.validate();
            for(auto placement:decodePlacements(resource.path,table)){auto world=concatenate(parent,placementMatrix(placement));if(leaf=="stageobjinfo")load(placement.name,world);
                else{assetCheck(result.placements.size()<100000,"Scenario placement budget exceeded");ScenarioPlacement record{placement,world,name,instance,parent};if(leaf=="startinfo"){int field=table.find("MarioNo");assetCheck(field>=0,"Player start lacks MarioNo");auto value=table.read(placement.row,size_t(field));assetCheck(std::holds_alternative<uint32_t>(value),"Player start MarioNo is not integer");record.startId=int32_t(std::get<uint32_t>(value));}result.placements.push_back(record);}}
        }stack.pop_back();
    };load(galaxy,identityMatrix());return result;
}
inline std::string scenarioActorKey(const ScenarioPlacement& record){return record.zone+"#"+std::to_string(record.zoneInstance)+":"+record.placement.table+":"+std::to_string(record.placement.row);}
inline const ScenarioPlacement* findScenarioStart(const ScenarioAssembly& assembly,const std::string& zone,int32_t id){
    const ScenarioPlacement* found=nullptr;for(const auto& record:assembly.placements)if(record.zone==zone&&record.startId==id){if(found&&found->zoneInstance!=record.zoneInstance)throw std::runtime_error("Start is ambiguous across repeated zone instances");if(!found)found=&record;}return found;
}
inline void scenarioStartTests(){
    ScenarioAssembly assembly;ScenarioPlacement one{};one.zone="zone";one.zoneInstance=3;one.startId=4;assembly.placements.push_back(one);one.startId=0;assembly.placements.push_back(one);
    assetCheck(findScenarioStart(assembly,"zone",0)==&assembly.placements[1]&&!findScenarioStart(assembly,"zone",9),"Scenario start ID lookup failed");one.zoneInstance=8;assembly.placements.push_back(one);bool rejected=false;try{findScenarioStart(assembly,"zone",0);}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Ambiguous start zone instance accepted");assetCheck(scenarioActorKey(assembly.placements[1])!=scenarioActorKey(assembly.placements[2]),"Repeated zone instances share an actor key");
}




