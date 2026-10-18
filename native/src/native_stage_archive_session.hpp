#pragma once
#include "native_disc_archive_loader.hpp"
#include "native_archive_jmap_tables.hpp"
#include <Game/System/ScenarioDataParser.hpp>
#include <Native/ScenarioSource.hpp>
// Session owns zone bridges so original pointers and table names stay valid.
class NativeStageArchiveSession : public NativeScenarioSource {
 NativeDiscArchiveLoader& loader;
 std::unique_ptr<NativeArchiveJMapTables> objectNames;
 std::map<std::string,std::unique_ptr<NativeArchiveJMapTables>> stages;
 std::map<std::string,std::unique_ptr<NativeArchiveJMapTables>> scenarioArchives;
 std::map<std::string,std::unique_ptr<ScenarioData>> scenarios;
 static std::string stageKey(const char* name){
  assetCheck(name && *name,"Empty native stage name");std::string key(name);
  for(unsigned char c:key)assetCheck((c>='A' && c<='Z') || (c>='a' && c<='z') || (c>='0' && c<='9') || c=='_',"Invalid native stage name");return key;
 }
public:
 std::vector<const ScenarioData*> scenarioList() override {
  std::vector<const ScenarioData*> result;
  for(const auto& file:loader.files())if(file.path.rfind("/StageData/",0)==0){
   auto separator=file.path.find('/',11);if(separator==std::string::npos)continue;auto name=file.path.substr(11,separator-11);
   if(file.path=="/StageData/"+name+"/"+name+"Scenario.arc")result.push_back(scenario(name.c_str()));
  }
  return result;
 }
 const ScenarioData* scenario(const char* name){
  auto key=stageKey(name);auto found=scenarios.find(key);if(found!=scenarios.end())return found->second.get();
  auto archive=std::make_unique<NativeArchiveJMapTables>(loader.mount("/StageData/"+key+"/"+key+"Scenario.arc"));
  auto data=archive->get("/ScenarioData.bcsv"),zones=archive->get("/ZoneList.bcsv");assetCheck(data && zones,"Native scenario archive missing tables");
  auto pending=std::make_unique<ScenarioData>(key.c_str(),*data,*zones);auto result=pending.get();
  scenarioArchives.emplace(key,std::move(archive));scenarios.emplace(key,std::move(pending));return result;
 }
 explicit NativeStageArchiveSession(NativeDiscArchiveLoader& source):loader(source){}
 const JMapInfo* objectNameTable(){
  if(!objectNames){auto pending=std::make_unique<NativeArchiveJMapTables>(loader.mount("/StageData/ObjNameTable.arc"));assetCheck(pending->get("/ObjNameTable.tbl")!=nullptr,"Object-name archive table missing");objectNames=std::move(pending);}
  return objectNames->get("/ObjNameTable.tbl");
 }
 NativeArchiveJMapTables* stage(const char* name){
  auto key=stageKey(name);
  auto found=stages.find(key);if(found!=stages.end())return found->second.get();
  auto bridge=std::make_unique<NativeArchiveJMapTables>(loader.mount("/StageData/"+key+".arc"));
  bridge->objectNameResolver=[this](){return objectNameTable();};
  bridge->stageResolver=[this](const char* child){return stage(child);};
  auto result=bridge.get();stages.emplace(key,std::move(bridge));return result;
 }
 NativeStageArchiveSession(const NativeStageArchiveSession&)=delete;
 NativeStageArchiveSession& operator=(const NativeStageArchiveSession&)=delete;
};




