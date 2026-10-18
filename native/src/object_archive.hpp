#pragma once
#include "factory_archives.hpp"
inline std::optional<std::string> resolveObjectArchive(WiiAssets& assets,const std::string& name){
    auto exists=[&](const std::string& candidate){for(const auto& file:assets.files)if(file.path==candidate)return true;return false;};
    if(auto primary=factoryPrimaryArchive(name)){std::string path="/ObjectData/"+std::string(primary)+".arc";if(exists(path))return path;}
    std::string direct="/ObjectData/"+name+".arc";if(exists(direct))return direct;return std::nullopt;
}
inline void factoryArchiveTests(){
    assetCheck(std::string(factoryPrimaryArchive("HammerHeadPackun"))=="PackunFlower","Factory primary archive mapping failed");
    assetCheck(std::string(factoryPrimaryArchive("hammerheadpackun"))=="PackunFlower","Factory archive case lookup failed");
    assetCheck(factoryPrimaryArchive("UnknownNativeTestObject")==nullptr,"Unknown factory object resolved");
}
