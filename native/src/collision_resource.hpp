#pragma once
#include "assets.hpp"
// PlanetMap initializes its main collision by model name; special surfaces are separate.
inline std::optional<std::string> selectMainCollision(const std::string& name,const std::vector<std::string>& resources){
    auto lower=[](std::string value){for(char& c:value)if(c>='A'&&c<='Z')c=char(c-'A'+'a');return value;};
    auto expected=lower(name);std::optional<std::string> match;
    for(const auto& path:resources)if(lower(std::filesystem::path(path).stem().string())==expected){assetCheck(!match,"Ambiguous named main collision");match=path;}
    if(match)return match;if(resources.size()==1)return resources.front();return std::nullopt;
}
inline void collisionResourceTests(){
    auto selected=selectMainCollision("BroadBeanPlanetC",{"/movelimit.kcl","/broadbeanplanetc.kcl"});assetCheck(selected&&*selected=="/broadbeanplanetc.kcl","Named main collision selection failed");
    assetCheck(!selectMainCollision("Unknown",{"/one.kcl","/two.kcl"}),"Ambiguous collision archive selected");assetCheck(!selectMainCollision("Unknown",{}),"Empty collision archive selected");
    assetCheck(selectMainCollision("Other",{"/single.kcl"}).value()=="/single.kcl","Single collision compatibility failed");
    bool rejected=false;try{selectMainCollision("Thing",{"/thing.kcl","/nested/THING.kcl"});}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Duplicate named collision accepted");
}
