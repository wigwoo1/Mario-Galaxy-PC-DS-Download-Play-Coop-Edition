#include <Game/System/GameDataGalaxyStorage.hpp>
#include <Game/System/BinaryDataContentAccessor.hpp>
#include <Game/System/GalaxyStatusAccessor.hpp>
#include <stdexcept>
GameDataSomeGalaxyStorage::GameDataSomeGalaxyStorage(const GalaxyStatusAccessor& rAccessor) {
    mGalaxyName = rAccessor.getName();
    mPowerStarNum = rAccessor.getPowerStarNum();
    resetAllData();
}
void GameDataSomeGalaxyStorage::resetAllData() {
    mPowerStarOwnedFlags = 0b00000000;
    mAlreadyVisitedFlags = 0b00000000;
    _A = 0;
    _B = 0;

    for (s32 idx = 0; idx < 8; idx++) {
        mMaxCoinNum[idx] = 0;
    }
}
s32 GameDataSomeGalaxyStorage::deserialize(const BinaryDataContentAccessor& a,const u8* data) {
    if (!data) return -1;
    u8* stars=(u8*)a.getPointer("mPowerStarFlag",(u8*)data);
    u8* visited=(u8*)a.getPointer("mFirstPlayFlag",(u8*)data);
    u8* coins=(u8*)a.getPointer("mMaxCoinNum",(u8*)data);
    if (coins && (a.getDataSize()<16 || coins-data>a.getDataSize()-16)) return -1;
    u16 decoded[8]{};
    if(coins) for(int i=0;i<8;++i) decoded[i]=u16((u16(coins[i*2])<<8)|coins[i*2+1]);
    mPowerStarOwnedFlags=stars?*stars:0;
    mAlreadyVisitedFlags=visited?*visited:0;
    for(int i=0;i<8;++i)mMaxCoinNum[i]=decoded[i];
    return stars&&visited&&coins?0:1;
}
void GameDataSomeGalaxyStorage::serialize(const BinaryDataContentAccessor& a,u8* data) const {
    if(!data) throw std::invalid_argument("Null galaxy record");
    u8* stars=(u8*)a.getPointer("mPowerStarFlag",data);
    u8* visited=(u8*)a.getPointer("mFirstPlayFlag",data);
    u8* coins=(u8*)a.getPointer("mMaxCoinNum",data);
    if(!stars||!visited||!coins||a.getDataSize()<16||coins-data>a.getDataSize()-16)
        throw std::length_error("Incomplete galaxy record fields");
    *stars=mPowerStarOwnedFlags;*visited=mAlreadyVisitedFlags;
    for(int i=0;i<8;++i){coins[i*2]=u8(mMaxCoinNum[i]>>8);coins[i*2+1]=u8(mMaxCoinNum[i]);}
}

