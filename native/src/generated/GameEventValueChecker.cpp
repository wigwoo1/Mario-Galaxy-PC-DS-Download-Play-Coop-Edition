#include <vector>
#include "Game/System/GameEventValueChecker.hpp"
#include "Game/System/FindingLuigiEventScheduler.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JSupport/JSUMemoryInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryOutputStream.hpp>

namespace {
    const GameEventValue cGameEventValueTable[] = {
        {"ペンギンレース[オーシャンリング]/hi", 0},
        {"ペンギンレース[オーシャンリング]/lo", 90 * 60},
        {"テレサレース[ファントム]/hi", 0},
        {"テレサレース[ファントム]/lo", 90 * 60},
        {"テレサレース[デスプロムナード]/hi", 0},
        {"テレサレース[デスプロムナード]/lo", 90 * 60},
        {"サーフィン[トライアル]/hi", 0},
        {"サーフィン[トライアル]/lo", 90 * 60},
        {"サーフィン[チャレンジ]/hi", 0},
        {"サーフィン[チャレンジ]/lo", 90 * 60},
        {"LibraryOpenNewStarCount", 1},
        {"絵本既読章", 0},
        {"MsgLedPattern", 1},
        {"LuigiEventState", FindingLuigiEventScheduler::STATE_NULL},
        {"WarpPodSaveBits", 0},
        {"TicoGalaxyAlreadyTalk", 0},
        {"MessageAlreadyRead", 0},
        {"MissPointForLetter", 0},
        {"MissNum", 0},
        {"Comet1Status", 0},
        {"Comet2Status", 0},
        {"Comet3Status", 0},
        {"Comet4Status", 0},
        {"Comet5Status", 0},
        {"Comet6Status", 0},
    };
}  // namespace

GameEventValueChecker::GameEventValueChecker() : mValues(), mNumValues() {
    mValues = new u16[ARRAY_SIZE(::cGameEventValueTable)];
    mNumValues = ARRAY_SIZE(::cGameEventValueTable);
    initializeData();
}

u32 GameEventValueChecker::getValue(const char* pName) const {
    return mValues[findIndex(pName)];
}

void GameEventValueChecker::setValue(const char* pName, u16 value) {
    mValues[findIndex(pName)] = value;
}

u32 GameEventValueChecker::makeHeaderHashCode() const {
    return getSignature();
}

u32 GameEventValueChecker::getSignature() const {
    return 'VLE1';
}

s32 GameEventValueChecker::serialize(u8* pData, u32 maxBufferSize) const {
 if(mNumValues<0 || u32(mNumValues)>0x7fffffff/4 || u32(mNumValues)>maxBufferSize/4 || (!pData&&mNumValues))return -1;
 for(s32 i=0;i<mNumValues;++i){
  const u16 hash=MR::getHashCode(::cGameEventValueTable[i].mName);
  pData[4*i]=u8(hash>>8);pData[4*i+1]=u8(hash);
  pData[4*i+2]=u8(mValues[i]>>8);pData[4*i+3]=u8(mValues[i]);
 }
 return mNumValues*4;
}

s32 GameEventValueChecker::deserialize(const u8* pData, u32 maxBufferSize) {
 if((maxBufferSize%4)!=0 || (!pData&&maxBufferSize) || mNumValues<0)return -1;
 std::vector<std::pair<s32,u16>> pending;bool unknown=false;
 for(u32 offset=0;offset<maxBufferSize;offset+=4){
  const u16 hash=(u16(pData[offset])<<8)|pData[offset+1];
  const u16 value=(u16(pData[offset+2])<<8)|pData[offset+3];
  const s32 index=findIndexFromHashCode(hash);
  if(index<0){unknown=true;continue;}
  if(index>=mNumValues)return -1;
  pending.emplace_back(index,value);
 }
 for(const auto& update:pending)mValues[update.first]=update.second;
 return unknown?1:0;
}

void GameEventValueChecker::initializeData() {
    for (s32 idx = 0; idx < mNumValues; idx++) {
        mValues[idx] = ::cGameEventValueTable[idx].mDefaultValue;
    }
}

s32 GameEventValueChecker::findIndex(const char* pName) const {
    for (s32 idx = 0; idx < mNumValues; idx++) {
        if (MR::isEqualString(pName, ::cGameEventValueTable[idx].mName)) {
            return idx;
        }
    }

    return -1;
}

s32 GameEventValueChecker::findIndexFromHashCode(u16 hash) const {
    for (s32 idx = 0; idx < mNumValues; idx++) {
        if (hash == static_cast< u16 >(MR::getHashCode(::cGameEventValueTable[idx].mName))) {
            return idx;
        }
    }

    return -1;
}

