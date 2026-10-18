#pragma once
#include "stage_data.hpp"
#include "galaxy_progress.hpp"
struct CampaignGalaxyDescriptor{std::string name;uint32_t stars=0;size_t scenarios=0;};
inline std::vector<CampaignGalaxyDescriptor> loadCampaignCatalog(WiiAssets& assets){
    std::vector<CampaignGalaxyDescriptor> result;
    for(const auto& file:assets.files){
        constexpr const char* root="/StageData/";if(file.path.rfind(root,0)!=0)continue;
        auto split=file.path.find('/',11);if(split==std::string::npos)continue;
        auto name=file.path.substr(11,split-11);if(name.empty()||file.path.substr(split+1)!=name+"Scenario.arc")continue;
        Rarc archive(assets.readFile(file.path));auto bytes=archive.readFile("/scenariodata.bcsv");StageTable table{BinaryView(bytes)};table.validate();
        int number=table.find("ScenarioNo"),star=table.find("PowerStarId");assetCheck(number>=0,"Campaign scenario metadata lacks ScenarioNo");
        std::map<uint32_t,uint32_t> rows;
        for(size_t row=0;row<table.rows;++row){auto value=table.read(row,size_t(number));assetCheck(std::holds_alternative<uint32_t>(value),"Campaign scenario number type mismatch");uint32_t stars=0;if(star>=0){auto id=table.read(row,size_t(star));assetCheck(std::holds_alternative<uint32_t>(id),"Campaign star ID type mismatch");stars=std::get<uint32_t>(id);}assetCheck(rows.emplace(std::get<uint32_t>(value),stars).second,"Duplicate campaign scenario number");}
        CampaignGalaxyDescriptor descriptor{name,0,table.rows};
        // Original ScenarioData::getPowerStarNum examines numbers 1 through row count.
        for(uint32_t scenario=1;scenario<=table.rows;++scenario){auto found=rows.find(scenario);if(found!=rows.end()&&found->second!=0)++descriptor.stars;}
        assetCheck(descriptor.stars<=8,"Campaign star count exceeds original progress storage");result.push_back(std::move(descriptor));
    }
    std::sort(result.begin(),result.end(),[](const auto& a,const auto& b){return a.name<b.name;});
    assetCheck(!result.empty(),"Disc has no campaign scenario archives");return result;
}
inline void initializeCampaignCatalog(NativeCampaignProgress& campaign,const std::vector<CampaignGalaxyDescriptor>& catalog){for(const auto& galaxy:catalog)if(galaxy.stars!=0)campaign.galaxy(galaxy.name,int(galaxy.stars));}
