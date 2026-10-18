#include <vector>
#include <cstring>
namespace {
u16 galaxyRead16(const u8* p){return u16((u16(p[0])<<8)|p[1]);}
bool validGalaxyHeader(const u8* p,u32 bytes){
 if(!p||bytes<4)return false;
 const u32 count=galaxyRead16(p),length=galaxyRead16(p+2);
 if(count>(bytes-4)/4||length==0)return false;
 for(u32 i=0;i<count;++i){
  const u16 hash=galaxyRead16(p+4+i*4),offset=galaxyRead16(p+6+i*4);
  if(offset>=length)return false;
  for(u32 j=0;j<i;++j)if(galaxyRead16(p+4+j*4)==hash)return false;
 }
 return true;
}
}
#include <stdexcept>
#include "Game/System/GameDataGalaxyStorage.hpp"
#include "Game/System/BinaryDataContentAccessor.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/GameDataFunction.hpp"
#include "Game/System/GameDataHolder.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/StringUtil.hpp"

namespace {
    inline s32 writeU16(void* pData, u16 value) {
        u8* bytes=static_cast<u8*>(pData); bytes[0]=u8(value>>8); bytes[1]=u8(value);
        return sizeof(value);
    }
};  // namespace

bool GameDataSomeScenarioAccessor::hasPowerStar() const {
    return mSomeGalaxyStorage->hasPowerStar(mScenarioNum - 1);
}

bool GameDataSomeScenarioAccessor::isAlreadyVisited() const {
    return mSomeGalaxyStorage->isAlreadyVisited(mScenarioNum - 1);
}

s32 GameDataSomeScenarioAccessor::getMaxCoinNum() const {
    return MR::clamp(mSomeGalaxyStorage->getMaxCoinNum(mScenarioNum - 1), 0, 999L);
}

void GameDataSomeScenarioAccessor::setPowerStarFlag(bool val) {
    setBitFlagAccordingToBool(&mSomeGalaxyStorage->mPowerStarOwnedFlags, val);
}

void GameDataSomeScenarioAccessor::setFlagAlreadyVisited(bool val) {
    setBitFlagAccordingToBool(&mSomeGalaxyStorage->mAlreadyVisitedFlags, val);
}

void GameDataSomeScenarioAccessor::updateMaxCoinNum(int coinNum) {
    if (getMaxCoinNum() < coinNum) {
        s32 scenarioNo = mScenarioNum - 1;
        u16 maxCoins = MR::clamp(coinNum, 0, 999L);
        mSomeGalaxyStorage->setMaxCoinNum(scenarioNo, maxCoins);
    }
}

void GameDataSomeScenarioAccessor::setBitFlagAccordingToBool(u8* pFlags, bool val) {
    if (val) {
        *pFlags |= (1 << (mScenarioNum - 1));
    } else {
        *pFlags &= ~(1 << (mScenarioNum - 1));
    }
}

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

s32 GameDataSomeGalaxyStorage::getPowerStarNumOwned() const {
    s32 numOwned = 0;
    for (s32 scenarioNo = 1; scenarioNo <= mPowerStarNum; scenarioNo++) {
        if (hasPowerStar(scenarioNo - 1)) {
            numOwned++;
        }
    }
    return numOwned;
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
GameDataAllGalaxyStorage::GameDataAllGalaxyStorage() : mHeaderSerializer(nullptr) {
    mHeaderSerializer = new BinaryDataContentHeaderSerializer(new u8[32], 32);
    mSomeGalaxyStorages.init(64);

    ScenarioDataIter iter = MR::makeBeginScenarioDataIter();
    while (!iter.isEnd()) {
        GalaxyStatusAccessor accessor = iter.makeAccessor();
        if (accessor.getPowerStarNum() != 0) {
            mSomeGalaxyStorages.push_back(new GameDataSomeGalaxyStorage(accessor));
        }
        iter.goNext();
    }
    initHeaderSerializer();
}

bool GameDataAllGalaxyStorage::isExistAccessor(const char* pGalaxyName, int scenarioNum) const {
    if (findIndex(pGalaxyName) < 0) {
        return false;
    }

    GalaxyStatusAccessor accessor = MR::makeGalaxyStatusAccessor(pGalaxyName);
    if (scenarioNum >= 1 && scenarioNum <= accessor.getScenarioNum()) {
        return true;
    }

    return false;
}

GameDataSomeScenarioAccessor GameDataAllGalaxyStorage::makeAccessor(const char* pGalaxyName, int scenarioNum) const {
    return GameDataSomeScenarioAccessor(mSomeGalaxyStorages[findIndex(pGalaxyName)], scenarioNum);
}

s32 GameDataAllGalaxyStorage::calcCurrentPowerStarNum() const {
    s32 powerStarNum = 0;
    for (s32 idx = 0; idx < getGalaxyNum(); idx++) {
        powerStarNum += mSomeGalaxyStorages[idx]->getPowerStarNumOwned();
    }
    return powerStarNum;
}

s32 GameDataAllGalaxyStorage::getPowerStarNumOwned(const char* pGalaxyName) const {
    return mSomeGalaxyStorages[findIndex(pGalaxyName)]->getPowerStarNumOwned();
}

s32 GameDataAllGalaxyStorage::getGalaxyNum() const {
    return mSomeGalaxyStorages.size();
}

GameDataSomeGalaxyStorage* GameDataAllGalaxyStorage::getGalaxyStorage(int index) {
    return mSomeGalaxyStorages[index];
}

u32 GameDataAllGalaxyStorage::makeHeaderHashCode() const {
    return 0xBF0640EE;
}

u32 GameDataAllGalaxyStorage::getSignature() const {
    return 'GALA';
}

s32 GameDataAllGalaxyStorage::deserialize(const u8* data,u32 size){
 if(!data||size<6||!validGalaxyHeader(data+2,size-2))return -1;
 const u32 records=galaxyRead16(data);
 BinaryDataContentAccessor accessor(const_cast<u8*>(data+2));
 const u32 header=accessor.getHeaderSize()+2,stride=accessor.getDataSize();
 if(header>size||records>(size-header)/stride)return -1;
 std::vector<GameDataSomeGalaxyStorage> pending;
 std::vector<int> indices;
 bool readError=false;
 for(u32 i=0;i<records;++i){
  const u8* record=data+header+i*stride;
  const u8* name=(u8*)accessor.getPointer("mGalaxyName",const_cast<u8*>(record));
  if(!name||stride<2||name-record>stride-2)return -1;
  const int index=findIndex(galaxyRead16(name));
  if(index<0){readError=true;continue;}
  for(int old:indices)if(old==index)return -1;
  pending.push_back(*mSomeGalaxyStorages[index]);indices.push_back(index);
  const int result=pending.back().deserialize(accessor,record);
  if(result<0)return -1;
  if(result==1)readError=true;
 }
 for(size_t i=0;i<pending.size();++i)*mSomeGalaxyStorages[indices[i]]=pending[i];
 return readError?1:0;
}
void GameDataAllGalaxyStorage::initializeData() {
    for (s32 idx = 0; idx < getGalaxyNum(); idx++) {
        getGalaxyStorage(idx)->resetAllData();
    }
}

void GameDataAllGalaxyStorage::initHeaderSerializer() {
    mHeaderSerializer->addAttribute("mGalaxyName", sizeof(u16));
    mHeaderSerializer->addAttribute("mPowerStarFlag", sizeof(u8));
    mHeaderSerializer->addAttribute("mFirstPlayFlag", sizeof(u8));
    mHeaderSerializer->addAttribute("mMaxCoinNum", sizeof(u16) * 8);
    mHeaderSerializer->flush();
}

s32 GameDataAllGalaxyStorage::findIndex(const char* pGalaxyName) const {
    for (s32 idx = 0; idx < getGalaxyNum(); idx++) {
        if (MR::isEqualString(pGalaxyName, mSomeGalaxyStorages[idx]->mGalaxyName)) {
            return idx;
        }
    }
    return -1;
}

s32 GameDataAllGalaxyStorage::findIndex(u16 galaxyNameHash) const {
    for (s32 idx = 0; idx < getGalaxyNum(); idx++) {
        if ((u16)MR::getHashCode(mSomeGalaxyStorages[idx]->mGalaxyName) == galaxyNameHash) {
            return idx;
        }
    }
    return -1;
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
s32 GameDataAllGalaxyStorage::serialize(u8* pData, u32 dataSize) const {
    const u32 header=getSerializer()->getHeaderSize(),stride=getSerializer()->getDataSize();
    const u32 count=getGalaxyNum();
    if(!pData||count>65535||header>dataSize||dataSize-header<2||stride==0||count>(dataSize-header-2)/stride) return -1;
    if(!validGalaxyHeader((u8*)getSerializer()->getBuffer(),header)) return -1;
    // Validate all output fields before writing any bytes.
    BinaryDataContentAccessor check((u8*)getSerializer()->getBuffer());
    std::vector<u8> probe(stride);
    u8* nameField=(u8*)check.getPointer("mGalaxyName",probe.data());
    if(!nameField||stride<2||nameField-probe.data()>stride-2)return -1;
    try{for(u32 i=0;i<count;++i)mSomeGalaxyStorages[i]->serialize(check,probe.data());}
    catch(const std::length_error&){return -1;}

    const char* name;
    s32 writeOffset = 0;
    writeOffset += ::writeU16(pData + writeOffset, getGalaxyNum());
    name = reinterpret_cast< const char* >(getSerializer()->getBuffer());
    MR::copyMemory(pData + writeOffset, name, getSerializer()->getHeaderSize());
    writeOffset += getSerializer()->getHeaderSize();

    BinaryDataContentAccessor accessor(static_cast< u8* >(getSerializer()->getBuffer()));
    for (s32 idx = 0; idx < getGalaxyNum(); idx++) {
        GameDataSomeGalaxyStorage* storage = mSomeGalaxyStorages[idx];
        name = "mGalaxyName";
        u8* record = pData + writeOffset;
        void* writePtr = accessor.getPointer(name, record);
        ::writeU16(writePtr, MR::getHashCode(storage->mGalaxyName));
        storage->serialize(accessor, record);
        writeOffset += getSerializer()->getDataSize();
    }

    return writeOffset;
}


