#pragma once
#include "stage_data.hpp"
struct ScenarioZone {std::string name;uint32_t layers;};
inline std::vector<ScenarioZone> loadScenarioLayers(WiiAssets& assets,const std::string& galaxy,uint32_t scenario){
    Rarc archive(assets.readFile("/StageData/"+galaxy+"/"+galaxy+"Scenario.arc"));auto scenarioBytes=archive.readFile("/scenariodata.bcsv"),zoneBytes=archive.readFile("/zonelist.bcsv");
    StageTable scenarios{BinaryView(scenarioBytes)},zones{BinaryView(zoneBytes)};scenarios.validate();zones.validate();int number=scenarios.find("ScenarioNo"),name=zones.find("ZoneName");assetCheck(number>=0&&name>=0,"Missing scenario fields");
    size_t selected=scenarios.rows;for(size_t row=0;row<scenarios.rows;++row){auto value=scenarios.read(row,size_t(number));assetCheck(std::holds_alternative<uint32_t>(value),"Scenario number type mismatch");if(std::get<uint32_t>(value)==scenario){assetCheck(selected==scenarios.rows,"Duplicate scenario number");selected=row;}}
    assetCheck(selected<scenarios.rows,"Scenario number not found");std::vector<ScenarioZone> result;
    for(size_t row=0;row<zones.rows;++row){auto value=zones.read(row,size_t(name));assetCheck(std::holds_alternative<std::string>(value),"Zone name type mismatch");std::string zone=std::get<std::string>(value);int field=scenarios.find(zone);assetCheck(field>=0,"Missing zone layer mask");auto mask=scenarios.read(selected,size_t(field));assetCheck(std::holds_alternative<uint32_t>(mask),"Zone mask type mismatch");result.push_back({zone,std::get<uint32_t>(mask)});}
    return result;
}
inline bool scenarioLayerEnabled(const std::string& layer,uint32_t mask){
    if(layer=="common")return true;assetCheck(layer.size()==6&&layer.substr(0,5)=="layer"&&layer[5]>='a'&&layer[5]<='p',"Unknown scenario layer name");return (mask&(1u<<unsigned(layer[5]-'a')))!=0;
}
inline void scenarioLayerTests(){assetCheck(scenarioLayerEnabled("common",0)&&scenarioLayerEnabled("layera",1)&&!scenarioLayerEnabled("layerb",1)&&scenarioLayerEnabled("layerp",0x8000),"Scenario layer bit mapping test");}
