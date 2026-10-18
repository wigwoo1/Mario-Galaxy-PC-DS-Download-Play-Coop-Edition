#include <vector>
#include "Game/System/GameEventFlagStorage.hpp"
#include "Game/System/GameEventFlag.hpp"
#include "Game/System/GameEventFlagTable.hpp"
#include "Game/Util/BitArray.hpp"
#include "Game/Util/HashUtil.hpp"
#include <JSystem/JSupport/JSUMemoryInputStream.hpp>
#include <JSystem/JSupport/JSUMemoryOutputStream.hpp>

GameEventFlagStorage::GameEventFlagStorage() : mFlagBitArray(nullptr) {
    mFlagBitArray = new MR::BitArray(GameEventFlagTable::getTableSize());
}

void GameEventFlagStorage::set(const GameEventFlag* pFlag, bool isOn) {
    mFlagBitArray->set(GameEventFlagTable::getIndex(pFlag), isOn);
}

bool GameEventFlagStorage::isOn(const GameEventFlag* pFlag) const {
    return mFlagBitArray->isOn(GameEventFlagTable::getIndex(pFlag));
}

u32 GameEventFlagStorage::getSignature() const {
    return 'FLG1';
}

s32 GameEventFlagStorage::serialize(u8* pData, u32 size) const {
 std::vector<u16> records;
 for(GameEventFlagIter iter=GameEventFlagTable::getBeginIter();!iter.isEnd();iter.goNext()){
  if((iter.getFlag()->mSaveFlag&1)!=0)continue;
  GameEventFlagAccessor accessor(iter.getFlag());
  const u16 hash=MR::getHashCode(accessor.getName())&0x7fff;
  records.push_back(hash|(isOn(iter.getFlag())?0x8000:0));
 }
 if(records.size()>0x7fffffff/2 || records.size()>size/2 || (!pData&&!records.empty()))return -1;
 for(size_t i=0;i<records.size();++i){pData[2*i]=u8(records[i]>>8);pData[2*i+1]=u8(records[i]);}
 return s32(records.size()*2);
}

s32 GameEventFlagStorage::deserialize(const u8* pData, u32 size) {
 if((size&1)!=0 || (!pData&&size))return -1;
 std::vector<std::pair<int,bool>> updates;
 bool unknown=false;
 for(u32 offset=0;offset<size;offset+=2){
  const u16 data=(u16(pData[offset])<<8)|pData[offset+1];
  const int index=GameEventFlagTable::getIndexFromHashCode(data&0x7fff);
  if(index<0){unknown=true;continue;}
  if(index>=mFlagBitArray->size())return -1;
  updates.emplace_back(index,(data&0x8000)!=0);
 }
 for(const auto& update:updates)mFlagBitArray->set(update.first,update.second);
 return unknown?1:0;
}

void GameEventFlagStorage::initializeData() {
    for (int i = 0; i < mFlagBitArray->size(); i++) {
        mFlagBitArray->set(i, false);
    }
}

u32 GameEventFlagStorage::makeHeaderHashCode() const {
    return MR::getHashCode("2bytes/flag");
}

