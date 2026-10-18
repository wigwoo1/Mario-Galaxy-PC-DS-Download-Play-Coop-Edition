#pragma once
#include "assets.hpp"
#include "Game/System/AlreadyDoneFlagInGalaxy.hpp"
#include "Game/System/GameDataTemporaryInGalaxy.hpp"
#include "Game/Util/JMapIdInfo.hpp"
#include <algorithm>
class NativePlacedInfoScope{public:NativePlacedInfoScope(const JMapInfoIter&,int);~NativePlacedInfoScope();NativePlacedInfoScope(const NativePlacedInfoScope&)=delete;NativePlacedInfoScope& operator=(const NativePlacedInfoScope&)=delete;};
class NativeStageMemory{
    static int checkedCapacity(int capacity){assetCheck(capacity>0&&capacity<=64,"Invalid stage memory capacity");return capacity;}
    JMapIdInfo restart{0,0};AlreadyDoneFlagInGalaxy flags;
public:
    GameDataTemporaryInGalaxy result;
    explicit NativeStageMemory(int capacity=64):flags(checkedCapacity(capacity)){result.mPlayerRestartIdInfo=&restart;result.mAlreadyDoneFlag=&flags;}
    NativeStageMemory(const NativeStageMemory&)=delete;NativeStageMemory& operator=(const NativeStageMemory&)=delete;
    const JMapIdInfo& restartInfo()const{return restart;}
    void setRestart(int start,int zone){assetCheck(start>=0&&zone>=0,"Invalid checkpoint start/zone");result.setPlayerRestartIdInfo(JMapIdInfo(start,zone));}
    void resetRestart(){result.resetPlayerRestartIdInfo();}
    uint32_t setup(const char* name,const JMapInfoIter& iter,int zone,uint32_t& value){
        assetCheck(name&&iter.isValid()&&zone>=0&&zone<=65535,"Invalid stage flag placement");NativePlacedInfoScope scope(iter,zone);
        s32 link=-1;iter.getValue<s32>("l_id",&link);assetCheck(link>=-1&&link<=65534,"Stage flag link ID outside original storage");
        AlreadyDoneInfo key;key.init(name,zone,link);auto found=std::find(flags.mDoneInfos.begin(),flags.mDoneInfos.end(),key);
        assetCheck(found!=flags.mDoneInfos.end()||flags.mDoneInfos.size()<flags.mDoneInfos.mArray.size(),"Stage flag capacity exceeded");
        return result.setupAlreadyDoneFlag(name,iter,&value);
    }
    void update(uint32_t index,uint32_t value){assetCheck(index<uint32_t(flags.mDoneInfos.size()),"Invalid stage flag index");result.updateAlreadyDoneFlag(int(index),value);}
    void clearFlags(){result.clearAlreadyDoneFlag();}
};
inline void stageMemoryTests(){
    struct Table{u32 count,fields,offset,stride;JMapItem item;s32 link;} table{1,1,28,4,{0,0xffffffff,0,0,0},7};
    uint32_t hash=0;for(char c:std::string("l_id"))hash=hash*31+uint8_t(c);table.item.mHash=hash;
    JMapInfo info;info.attach(&table);JMapInfoIter iter(&info,0);NativeStageMemory memory(2);uint32_t value=99;
    auto first=memory.setup("event",iter,0,value);assetCheck(first==0&&value==0,"Original stage flag initial state mismatch");memory.update(first,1);
    auto again=memory.setup("event",iter,0,value);assetCheck(again==first&&value==1,"Original stage flag did not retain completed event");
    auto other=memory.setup("event",iter,1,value);assetCheck(other==1&&value==0,"Original stage flag zone identity mismatch");
    bool rejected=false;try{memory.setup("other",iter,0,value);}catch(const std::exception&){rejected=true;}assetCheck(rejected,"Stage flag capacity overflow accepted");
    memory.setRestart(5,2);assetCheck(memory.restartInfo()==JMapIdInfo(5,2),"Original checkpoint assignment mismatch");memory.result.resetStageResultParam();assetCheck(memory.setup("event",iter,0,value)==0&&value==1&&memory.restartInfo()==JMapIdInfo(5,2),"Result reset discarded retry state");
    memory.resetRestart();assetCheck(memory.restartInfo()==JMapIdInfo(0,0),"Original checkpoint reset mismatch");memory.clearFlags();assetCheck(memory.setup("event",iter,0,value)==0&&value==0,"Original stage flags not cleared for new stage");
}

