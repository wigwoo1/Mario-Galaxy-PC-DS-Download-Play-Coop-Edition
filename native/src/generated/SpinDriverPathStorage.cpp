#include <vector>
#include <cstring>
#include "Game/System/SpinDriverPathStorage.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include "Game/System/ScenarioDataParser.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JSupport/JSUMemoryInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryOutputStream.hpp>

enum SpinDriverDataFlag {
    SpinDriverDataFlag_Interrupted = 0b10000000,
    SpinDriverDataFlag_Complete = 0b01000000,
    SpinDriverDataId_Mask = 0b00111111,
};

enum SpinDriverScenarioDataFlag {
    SpinDriverScenarioDataFlag_ZoneDataStart = 0b11000000,
    SpinDriverScenarioDataFlag_ZoneIdMask = 0b00111111,
    SpinDriverScenarioDataFlag_EndOfData = 0b11111111,
};

void SpinDriverPathStorage_FORCE_MATCHSDATA2() {
    f32 v1 = 1.0f;
    f32 v2 = 0.0f;
}

SpinDriverPathStorageOne::SpinDriverPathStorageOne() : mZoneId(-1), mSpinDriverId(-1), mDrawRange(-1.0f) {
    reset();
}

void SpinDriverPathStorageOne::reset() {
    mZoneId = -1;
    mSpinDriverId = -1;
    mDrawRange = -1.0f;
}

f32 SpinDriverPathStorageOne::registerToUse(int zoneId, int spinDriverId) {
    mZoneId = zoneId;
    mSpinDriverId = spinDriverId;
    mDrawRange = 0.0f;
    return mDrawRange;
}

void SpinDriverPathStorageOne::updateValue(f32 drawRange) {
    mDrawRange = drawRange;
}

s32 SpinDriverPathStorageOne::serialize(u8* pData) const {
    if (MR::isNearZero(mDrawRange)) {
        return 0;
    }

    pData[0] = mSpinDriverId & SpinDriverDataId_Mask | SpinDriverDataFlag_Interrupted;

    s32 writeSize;
    s32 drawRange = 256.0f * getDrawRange();
    if (drawRange <= 0) {
        writeSize = 0;
    } else {
        u8 val = drawRange >= 256 ? 0 : (u8)drawRange;

        JSUMemoryOutputStream stream(pData + 1, sizeof(u8));
        stream.writeU8(val);
        writeSize = stream.mPosition;
    }
    return writeSize + 1;
}

s32 SpinDriverPathStorageOne::deserialize(const u8* pData, int zoneId) {
    s32 readSize = 0;
    if ((*(pData + 0) & SpinDriverDataFlag_Interrupted) == 0) {
        mDrawRange = (*(pData + 0) & SpinDriverDataFlag_Complete) != 0 ? 1.0f : 0.0f;
    } else {
        u8 drawRange = 0;
        JSUMemoryInputStream stream(pData + 1, sizeof(u8));
        stream.read(&drawRange, sizeof(u8));

        mDrawRange = 0.0f;

        if (drawRange == 0) {
            mDrawRange = 1.0f;
        } else {
            mDrawRange = drawRange * getDrawRangeScale();
        }

        readSize = stream.mPosition;
    }

    mZoneId = zoneId;
    mSpinDriverId = pData[0] & SpinDriverDataId_Mask;
    return readSize + 1;
}

SpinDriverPathStorageScenario::SpinDriverPathStorageScenario() {
    mOneStorage.init(16);
    resetAllData();
}

void SpinDriverPathStorageScenario::resetAllData() {
    for (s32 idx = 0; idx < mOneStorage.size(); idx++) {
        mOneStorage[idx].reset();
    }
    mOneStorage.clear();
}

s32 SpinDriverPathStorageScenario::setup(int zoneId, int spinDriverId, f32* pDrawRange) {
    if (spinDriverId + 1 <= 1U) {
        return -1;
    }

    for (s32 idx = 0; idx < mOneStorage.size(); idx++) {
        if (((0.0f <= mOneStorage[idx].mDrawRange) != false) && zoneId == mOneStorage[idx].mZoneId &&
            spinDriverId == mOneStorage[idx].mSpinDriverId) {
            *pDrawRange = mOneStorage[idx].mDrawRange;
            return idx;
        }
    }

    SpinDriverPathStorageOne storage;
    storage.registerToUse(zoneId, spinDriverId);
    mOneStorage.push_back(storage);
    *pDrawRange = 0.0f;
    return mOneStorage.size() - 1;
}

void SpinDriverPathStorageScenario::updateValue(int spinDriverIndex, f32 drawRange) {
    mOneStorage[spinDriverIndex].updateValue(drawRange);
}

s32 SpinDriverPathStorageScenario::serialize(u8* pData, u32 dataSize) const {
 std::vector<u8> bytes(2);
 for(int zone=0;zone<=calcMaxZoneId();++zone){
  bool header=false;
  for(s32 i=0;i<mOneStorage.size();++i){
   const auto& one=mOneStorage[i];if(one.mZoneId!=zone)continue;
   if(zone>62 || one.mSpinDriverId<0 || one.mSpinDriverId>63)return -1;
   if(!header){bytes.push_back(u8(0xc0|zone));header=true;}
   u8 record[2]={};const s32 count=one.serialize(record);
   if(count<0||count>2)return -1;
   bytes.insert(bytes.end(),record,record+count);
  }
 }
 bytes.push_back(0xff);
 if(bytes.size()>65535 || bytes.size()>dataSize || !pData)return -1;
 bytes[0]=u8(bytes.size()>>8);bytes[1]=u8(bytes.size());
 std::memcpy(pData,bytes.data(),bytes.size());return s32(bytes.size());
}

s32 SpinDriverPathStorageScenario::deserialize(const u8* pData, u32 dataSize) {
 if(!pData || dataSize<3)return -1;
 const u32 block=(u32(pData[0])<<8)|pData[1];
 if(block<3||block>dataSize)return -1;
 std::vector<SpinDriverPathStorageOne> pending;int zone=-1;
 for(u32 offset=2;offset<block;){
  const u8 token=pData[offset];
  if(token==0xff){
   if(offset+1!=block)return -1;
   resetAllData();for(const auto& one:pending)mOneStorage.push_back(one);
   return s32(block);
  }
  if((token&0xc0)==0xc0){zone=token&0x3f;++offset;continue;}
  if(zone<0 || pending.size()>=16)return -1;
  const u32 count=(token&0x80)?2:1;
  if(count>block-offset)return -1;
  SpinDriverPathStorageOne one;one.deserialize(pData+offset,zone);
  pending.push_back(one);offset+=count;
 }
 return -1;
}

s32 SpinDriverPathStorageScenario::calcMaxZoneId() const {
    s32 maxId = 0;
    for (s32 idx = 0; idx < mOneStorage.size(); idx++) {
        if (maxId < mOneStorage[idx].mZoneId) {
            maxId = mOneStorage[idx].mZoneId;
        }
    }
    return maxId;
}

SpinDriverPathStorageGalaxy::SpinDriverPathStorageGalaxy(const GalaxyStatusAccessor& rAccessor)
    : mScenarioStorage(nullptr), mNumScenarios(0), mGalaxyName(nullptr) {
    s32 numScenarios = rAccessor.getScenarioNum();
    mScenarioStorage = new SpinDriverPathStorageScenario[rAccessor.getScenarioNum()];
    mNumScenarios = numScenarios;
    mGalaxyName = rAccessor.getName();
    resetAllData();
}

void SpinDriverPathStorageGalaxy::resetAllData() {
    for (s32 idx = 0; idx < mNumScenarios; idx++) {
        mScenarioStorage[idx].resetAllData();
    }
}

s32 SpinDriverPathStorageGalaxy::setup(int scenarioNo, int zoneId, int spinDriverId, f32* pDrawRange) {
    return getScenarioStorage(scenarioNo).setup(zoneId, spinDriverId, pDrawRange);
}

void SpinDriverPathStorageGalaxy::updateValue(int scenarioNo, int spinDriverIndex, f32 drawRange) {
    getScenarioStorage(scenarioNo).updateValue(spinDriverIndex, drawRange);
}

s32 SpinDriverPathStorageGalaxy::serialize(u8* pData, u32 maxBufferSize) const {
 if(mNumScenarios<0||mNumScenarios>255||!pData)return -1;
 std::vector<u8> bytes(6);
 for(s32 i=0;i<mNumScenarios;++i){
  u8 scenario[64];const s32 count=mScenarioStorage[i].serialize(scenario,sizeof(scenario));
  if(count<0)return -1;
  bytes.insert(bytes.end(),scenario,scenario+count);
 }
 if(bytes.size()>65535||bytes.size()>maxBufferSize)return -1;
 const u16 hash=MR::getHashCode(mGalaxyName);
 bytes[0]=u8(hash>>8);bytes[1]=u8(hash);bytes[2]=u8(bytes.size()>>8);bytes[3]=u8(bytes.size());bytes[4]=u8(mNumScenarios);
 std::memcpy(pData,bytes.data(),bytes.size());return s32(bytes.size());
}

s32 SpinDriverPathStorageGalaxy::deserialize(const u8* pData, u32 maxBufferSize) {
 if(!pData||maxBufferSize<6)return -1;
 const u16 hash=(u16(pData[0])<<8)|pData[1];
 const u32 block=(u32(pData[2])<<8)|pData[3];
 if(block<6||block>maxBufferSize)return -1;
 if(hash!=u16(MR::getHashCode(mGalaxyName)))return s32(block);
 const u32 count=pData[4];
 std::vector<std::vector<SpinDriverPathStorageOne>> pending(count);
 u32 offset=6;
 for(u32 i=0;i<count;++i){
  SpinDriverPathStorageScenario scenario;
  const s32 consumed=scenario.deserialize(pData+offset,block-offset);
  if(consumed<0)return -1;
  for(s32 j=0;j<scenario.mOneStorage.size();++j)pending[i].push_back(scenario.mOneStorage[j]);
  offset+=u32(consumed);
 }
 if(offset!=block)return -1;
 if(count!=u32(mNumScenarios)){resetAllData();return s32(block);}
 for(s32 i=0;i<mNumScenarios;++i){
  mScenarioStorage[i].resetAllData();
  for(const auto& one:pending[i])mScenarioStorage[i].mOneStorage.push_back(one);
 }
 return s32(block);
}

SpinDriverPathStorage::SpinDriverPathStorage() {
    mGalaxyStorage.init(50);

    ScenarioDataIter iter = MR::makeBeginScenarioDataIter();
    while (!iter.isEnd()) {
        GalaxyStatusAccessor accessor = iter.makeAccessor();
        mGalaxyStorage.push_back(new SpinDriverPathStorageGalaxy(accessor));
        iter.goNext();
    }
}

void SpinDriverPathStorage::initializeData() {
    for (s32 idx = 0; idx < mGalaxyStorage.size(); idx++) {
        mGalaxyStorage[idx]->resetAllData();
    }
}

s32 SpinDriverPathStorage::setup(const char* pGalaxyName, int scenarioNo, int zoneId, int spinDriverId, f32* pDrawRange) {
    return findFromGalaxy(pGalaxyName)->getScenarioStorage(scenarioNo).setup(zoneId, spinDriverId, pDrawRange);
}

void SpinDriverPathStorage::updateValue(const char* pGalaxyName, int scenarioNo, int spinDriverIndex, f32 drawRange) {
    findFromGalaxy(pGalaxyName)->updateValue(scenarioNo, spinDriverIndex, drawRange);
}

u32 SpinDriverPathStorage::makeHeaderHashCode() const {
    return 0x12345679;
}

u32 SpinDriverPathStorage::getSignature() const {
    return 'SPN1';
}

s32 SpinDriverPathStorage::serialize(u8* pData, u32 maxBufferSize) const {
 if(mGalaxyStorage.size()>255||!pData)return -1;
 std::vector<u8> bytes(1,u8(mGalaxyStorage.size()));
 for(s32 i=0;i<mGalaxyStorage.size();++i){
  std::vector<u8> block(65535);
  const s32 count=mGalaxyStorage[i]->serialize(block.data(),u32(block.size()));
  if(count<0)return -1;
  bytes.insert(bytes.end(),block.begin(),block.begin()+count);
 }
 if(bytes.size()>maxBufferSize||bytes.size()>0x7fffffff)return -1;
 std::memcpy(pData,bytes.data(),bytes.size());return s32(bytes.size());
}

s32 SpinDriverPathStorage::deserialize(const u8* pData, u32 maxBufferSize) {
 if(!pData||maxBufferSize<1)return -1;
 struct Update {SpinDriverPathStorageGalaxy* galaxy;std::vector<std::vector<SpinDriverPathStorageOne>> scenarios;};
 std::vector<Update> updates;u32 offset=1;bool unknown=false;
 for(u32 i=0;i<pData[0];++i){
  if(maxBufferSize-offset<6)return -1;
  const u8* block=pData+offset;
  const u16 hash=(u16(block[0])<<8)|block[1];
  const u32 size=(u32(block[2])<<8)|block[3];
  if(size<6||size>maxBufferSize-offset)return -1;
  auto* galaxy=findFromHashCode(hash);
  if(!galaxy){unknown=true;offset+=size;continue;}
  Update update{galaxy,{}};update.scenarios.resize(block[4]);
  u32 pos=6;
  for(u32 j=0;j<block[4];++j){
   SpinDriverPathStorageScenario scenario;
   const s32 consumed=scenario.deserialize(block+pos,size-pos);
   if(consumed<0)return -1;
   for(s32 k=0;k<scenario.mOneStorage.size();++k)update.scenarios[j].push_back(scenario.mOneStorage[k]);
   pos+=u32(consumed);
  }
  if(pos!=size)return -1;
  updates.push_back(std::move(update));offset+=size;
 }
 for(const auto& update:updates){
  update.galaxy->resetAllData();
  if(update.scenarios.size()!=u32(update.galaxy->mNumScenarios))continue;
  for(s32 i=0;i<update.galaxy->mNumScenarios;++i)for(const auto& one:update.scenarios[i])update.galaxy->mScenarioStorage[i].mOneStorage.push_back(one);
 }
 return unknown?1:0;
}

SpinDriverPathStorageGalaxy* SpinDriverPathStorage::findFromGalaxy(const char* pGalaxyName) {
    for (s32 idx = 0; idx < mGalaxyStorage.size(); idx++) {
        if (MR::isEqualString(mGalaxyStorage[idx]->mGalaxyName, pGalaxyName)) {
            return mGalaxyStorage[idx];
        }
    }
    return nullptr;
}

SpinDriverPathStorageGalaxy* SpinDriverPathStorage::findFromHashCode(u16 galaxyNameHash) {
    for (s32 idx = 0; idx < mGalaxyStorage.size(); idx++) {
        if ((u16)MR::getHashCode(mGalaxyStorage[idx]->mGalaxyName) == galaxyNameHash) {
            return mGalaxyStorage[idx];
        }
    }
    return nullptr;
}

