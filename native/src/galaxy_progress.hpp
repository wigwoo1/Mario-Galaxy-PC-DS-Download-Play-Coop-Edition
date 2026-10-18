#pragma once
#include "assets.hpp"
#include "Game/System/GameDataGalaxyStorage.hpp"
#include <memory>
#include <map>
#include <set>
#include "save_chunks.hpp"
std::unique_ptr<GameDataSomeGalaxyStorage> createNativeGalaxyProgress(const char*,int);
class NativeGalaxyProgress{std::string name;std::unique_ptr<GameDataSomeGalaxyStorage> storage;public:NativeGalaxyProgress(std::string galaxy,int stars):name(std::move(galaxy)),storage(createNativeGalaxyProgress(name.c_str(),stars)){}NativeGalaxyProgress(const NativeGalaxyProgress&)=delete;NativeGalaxyProgress& operator=(const NativeGalaxyProgress&)=delete;GameDataSomeScenarioAccessor scenario(int number){assetCheck(number>=1&&number<=8,"Invalid progress scenario");return {storage.get(),number};}int starsOwned()const{return storage->getPowerStarNumOwned();}
// Original field order, with explicit Wii endian encoding rather than host casts.
std::vector<uint8_t> saveFields()const{
    std::vector<uint8_t> bytes{storage->mPowerStarOwnedFlags,storage->mAlreadyVisitedFlags};
    for(auto coins:storage->mMaxCoinNum){bytes.push_back(uint8_t(coins>>8));bytes.push_back(uint8_t(coins));}
    return bytes;
}
bool loadFields(const std::vector<uint8_t>& bytes){
    if(bytes.size()!=18)return false;
    std::array<uint16_t,8> coins{};
    for(size_t i=0;i<coins.size();++i)coins[i]=uint16_t(uint16_t(bytes[2+i*2])<<8|bytes[3+i*2]);
    storage->mPowerStarOwnedFlags=bytes[0];storage->mAlreadyVisitedFlags=bytes[1];
    for(size_t i=0;i<coins.size();++i)storage->mMaxCoinNum[i]=coins[i];
    return true;
}};
inline void galaxyProgressTests(){NativeGalaxyProgress progress("test",8);auto first=progress.scenario(1),last=progress.scenario(8);assetCheck(!first.hasPowerStar()&&!last.isAlreadyVisited(),"Galaxy progress not reset");first.setPowerStarFlag(true);last.setPowerStarFlag(true);last.setFlagAlreadyVisited(true);assetCheck(progress.starsOwned()==2&&last.isAlreadyVisited(),"Original scenario flags mismatch");first.setPowerStarFlag(false);assetCheck(progress.starsOwned()==1&&last.hasPowerStar(),"Scenario flag clear leaked across bits");last.updateMaxCoinNum(2000);last.updateMaxCoinNum(2);assetCheck(last.getMaxCoinNum()==999,"Original best coin clamp/monotonic policy mismatch");bool rejected=false;try{progress.scenario(9);}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Ninth progress scenario accepted");}

// Session-owned registry; scene runtimes share records without owning their lifetime.
inline uint16_t progressNameHash(const std::string& name){uint32_t hash=0;for(unsigned char c:name)hash=hash*31+c;return uint16_t(hash);}
class NativeCampaignProgress : public BinaryDataChunkBase {
    struct Entry { int stars; std::shared_ptr<NativeGalaxyProgress> progress; };
    std::map<std::string,Entry> galaxies;
public:
    u32 makeHeaderHashCode()const override{return 0xBF0640EE;}
    u32 getSignature()const override{return 0x47414C41;}
    void initializeData()override{for(auto& [name,entry]:galaxies)entry.progress->loadFields(std::vector<uint8_t>(18));}
    s32 serialize(u8* output,u32 capacity)const override{
        size_t size=22+20*galaxies.size();if(!output||capacity<size)return -1;
        std::vector<uint8_t> bytes;auto half=[&](uint16_t value){bytes.push_back(uint8_t(value>>8));bytes.push_back(uint8_t(value));};
        half(uint16_t(galaxies.size()));half(4);half(20);
        const std::array<std::pair<const char*,uint16_t>,4> fields{{{"mGalaxyName",0},{"mPowerStarFlag",2},{"mFirstPlayFlag",3},{"mMaxCoinNum",4}}};
        for(auto field:fields){half(progressNameHash(field.first));half(field.second);}
        for(const auto& [name,entry]:galaxies){half(progressNameHash(name));auto payload=entry.progress->saveFields();bytes.insert(bytes.end(),payload.begin(),payload.end());}
        std::copy(bytes.begin(),bytes.end(),output);return s32(bytes.size());
    }
    s32 deserialize(const u8* input,u32 size)override{
        if(!input||size<6)return -1;
        auto half=[&](size_t offset){return uint16_t(uint16_t(input[offset])<<8|input[offset+1]);};
        size_t count=half(0),attributes=half(2),recordSize=half(4);
        if(count>64||attributes>64||recordSize<20)return -1;
        size_t headerSize=6+attributes*4;if(headerSize>size||count>(size-headerSize)/recordSize)return -1;
        const std::array<const char*,4> names{{"mGalaxyName","mPowerStarFlag","mFirstPlayFlag","mMaxCoinNum"}};
        std::array<size_t,4> offsets{};std::array<bool,4> seen{};
        for(size_t i=0;i<attributes;++i){auto hash=half(6+i*4);size_t offset=half(8+i*4);for(size_t f=0;f<4;++f)if(hash==progressNameHash(names[f])){size_t width=f==0?2:f==3?16:1;if(seen[f]||offset>recordSize||width>recordSize-offset)return -1;seen[f]=true;offsets[f]=offset;}}
        for(bool present:seen)if(!present)return -1;
        for(size_t a=0;a<4;++a)for(size_t b=a+1;b<4;++b){size_t aw=a==0?2:a==3?16:1,bw=b==0?2:b==3?16:1;if(offsets[a]<offsets[b]+bw&&offsets[b]<offsets[a]+aw)return -1;}
        struct Restore{std::shared_ptr<NativeGalaxyProgress> target;std::vector<uint8_t> fields;};std::vector<Restore> pending;std::set<uint16_t> records;
        for(size_t i=0;i<count;++i){size_t base=headerSize+i*recordSize;auto hash=half(base+offsets[0]);if(!records.insert(hash).second)return -1;
            std::shared_ptr<NativeGalaxyProgress> target;for(const auto& [name,entry]:galaxies)if(progressNameHash(name)==hash){target=entry.progress;break;}if(!target)return -1;
            std::vector<uint8_t> fields{input[base+offsets[1]],input[base+offsets[2]]};fields.insert(fields.end(),input+base+offsets[3],input+base+offsets[3]+16);pending.push_back({target,std::move(fields)});
        }
        for(auto& item:pending)item.target->loadFields(item.fields);return 0;
    }
    std::shared_ptr<NativeGalaxyProgress> galaxy(const std::string& name,int stars){
        assetCheck(!name.empty()&&stars>=0&&stars<=8,"Invalid campaign galaxy descriptor");
        auto found=galaxies.find(name);
        if(found!=galaxies.end()){assetCheck(found->second.stars==stars,"Campaign galaxy metadata changed");return found->second.progress;}
        assetCheck(galaxies.size()<64,"Campaign galaxy capacity exceeded");for(const auto& [existing,entry]:galaxies)assetCheck(progressNameHash(existing)!=progressNameHash(name),"Ambiguous Wii galaxy name hash");
        auto progress=std::make_shared<NativeGalaxyProgress>(name,stars);
        galaxies.emplace(name,Entry{stars,progress});return progress;
    }
};
inline void campaignProgressTests(){
    NativeCampaignProgress campaign;
    {auto scene=campaign.galaxy("first",2);scene->scenario(1).setFlagAlreadyVisited(true);scene->scenario(1).setPowerStarFlag(true);scene->scenario(1).updateMaxCoinNum(42);}
    auto reentered=campaign.galaxy("first",2);auto other=campaign.galaxy("second",0);
    assetCheck(reentered->scenario(1).isAlreadyVisited()&&reentered->scenario(1).hasPowerStar()&&reentered->scenario(1).getMaxCoinNum()==42,"Scene reentry lost campaign progress");
    assetCheck(!other->scenario(1).hasPowerStar()&&!reentered->scenario(2).hasPowerStar(),"Campaign progress leaked between galaxies/scenarios");
    bool rejected=false;try{campaign.galaxy("first",3);}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Campaign accepted inconsistent galaxy metadata");
}
inline void galaxyProgressFieldTests(){
    NativeGalaxyProgress original("save",8);original.scenario(1).setPowerStarFlag(true);original.scenario(8).setFlagAlreadyVisited(true);original.scenario(1).updateMaxCoinNum(300);original.scenario(8).updateMaxCoinNum(999);
    auto bytes=original.saveFields();assetCheck(bytes.size()==18&&bytes[0]==1&&bytes[1]==128&&bytes[2]==1&&bytes[3]==44&&bytes[16]==3&&bytes[17]==231,"Galaxy progress Wii field bytes mismatch");
    NativeGalaxyProgress restored("save",8);assetCheck(restored.loadFields(bytes)&&restored.scenario(1).hasPowerStar()&&restored.scenario(8).isAlreadyVisited()&&restored.scenario(1).getMaxCoinNum()==300&&restored.scenario(8).getMaxCoinNum()==999,"Galaxy progress field restore mismatch");
    bytes.pop_back();assetCheck(!restored.loadFields(bytes)&&restored.scenario(1).hasPowerStar()&&restored.scenario(1).getMaxCoinNum()==300,"Truncated galaxy fields changed progress");
}
inline void campaignSaveTests(){
    NativeCampaignProgress source;auto first=source.galaxy("first",2);source.galaxy("second",1)->scenario(1).setFlagAlreadyVisited(true);first->scenario(2).setPowerStarFlag(true);first->scenario(2).updateMaxCoinNum(321);
    auto bytes=encodeSaveChunks({&source});assetCheck(bytes.size()==78&&be32(bytes.data()+4)==0x47414C41&&be32(bytes.data()+8)==0xBF0640EE&&bytes[17]==2&&bytes[19]==4&&bytes[21]==20,"GALA chunk header/layout mismatch");
    NativeCampaignProgress restored;auto target=restored.galaxy("first",2);auto second=restored.galaxy("second",1);assetCheck(decodeSaveChunks(bytes,{&restored})&&target->scenario(2).hasPowerStar()&&target->scenario(2).getMaxCoinNum()==321&&second->scenario(1).isAlreadyVisited(),"GALA session restore mismatch");
    auto truncated=bytes;truncated.pop_back();assetCheck(!decodeSaveChunks(truncated,{&restored})&&target->scenario(2).hasPowerStar(),"Truncated GALA changed progress");
    auto bad=bytes;bad[24]=255;bad[25]=255;assetCheck(!decodeSaveChunks(bad,{&restored})&&target->scenario(2).getMaxCoinNum()==321,"Invalid GALA field offset changed progress");
    auto overlap=bytes;overlap[28]=0;overlap[29]=0;assetCheck(!decodeSaveChunks(overlap,{&restored})&&target->scenario(2).hasPowerStar(),"Overlapping GALA attributes changed progress");
    NativeCampaignProgress collisions;collisions.galaxy("Aa",1);bool collisionRejected=false;try{collisions.galaxy("BB",1);}catch(const std::exception&){collisionRejected=true;}assetCheck(collisionRejected,"Ambiguous Wii galaxy hash accepted");
    restored.initializeData();assetCheck(!target->scenario(2).hasPowerStar()&&!second->scenario(1).isAlreadyVisited(),"GALA reset broke shared runtime records");
    NativeCampaignProgress missing;missing.galaxy("first",2);assetCheck(!decodeSaveChunks(bytes,{&missing})&&!missing.galaxy("first",2)->scenario(2).hasPowerStar(),"Unknown GALA galaxy partially changed session");
}

