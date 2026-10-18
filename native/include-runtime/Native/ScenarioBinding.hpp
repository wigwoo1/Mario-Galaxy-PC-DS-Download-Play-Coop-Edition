#pragma once
#include <Game/System/ScenarioDataParser.hpp>
#include <string>
#include <stdexcept>
struct NativeScenarioBinding;
inline NativeScenarioBinding* nativeScenarioBinding=nullptr;
struct NativeScenarioBinding {
 ScenarioDataParser* parser;
 std::string galaxyName;
 NativeScenarioBinding* previous;
 NativeScenarioBinding(ScenarioDataParser& source,const char* galaxy):parser(&source),galaxyName(galaxy?galaxy:""),previous(nativeScenarioBinding){
  if(!parser->getScenarioData(galaxyName.c_str()))throw std::runtime_error("Native scenario galaxy unavailable");nativeScenarioBinding=this;
 }
 ~NativeScenarioBinding(){nativeScenarioBinding=previous;}
 NativeScenarioBinding(const NativeScenarioBinding&)=delete;
 NativeScenarioBinding& operator=(const NativeScenarioBinding&)=delete;
};
