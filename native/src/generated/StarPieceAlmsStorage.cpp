#include "Game/System/StarPieceAlmsStorage.hpp"
#include "Game/Util/HashUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MemoryUtil.hpp"

namespace {
    const static u32 sMaxValueArray[] = {32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 400, 400, 600, 1200, 1600, 1000, 800, 32767};
};  // namespace

StarPieceAlmsStorage::StarPieceAlmsStorage() : mValueArray() {
    mValueArray = new u16[16];

    for (s32 idx = 0; idx < 16; idx++) {
        mValueArray[idx] = 0;
    }
}

s32 StarPieceAlmsStorage::getValue(int idx) const {
    return mValueArray[idx];
}

s32 StarPieceAlmsStorage::getMaxValue(int idx) const {
    return ::sMaxValueArray[idx];
}

void StarPieceAlmsStorage::addValue(int idx, int add) {
    s32 value = mValueArray[idx];
    s32 maxValue = ::sMaxValueArray[idx];

    mValueArray[idx] = MR::clamp(value + add, 0, maxValue);
}

u32 StarPieceAlmsStorage::makeHeaderHashCode() const {
    return MR::getHashCode("StarPieceAlmsStorage") << 5;
}

u32 StarPieceAlmsStorage::getSignature() const {
    return 'PCE1';
}

s32 StarPieceAlmsStorage::serialize(u8* pData, u32 size) const {
 if(!pData || size<32)return -1;
 for(int i=0;i<16;++i){pData[2*i]=u8(mValueArray[i]>>8);pData[2*i+1]=u8(mValueArray[i]);}
 return 32;
}

s32 StarPieceAlmsStorage::deserialize(const u8* pData, u32 size) {
 if(!pData || size<32)return -1;
 for(int i=0;i<16;++i)mValueArray[i]=(u16(pData[2*i])<<8)|pData[2*i+1];
 return 0;
}

void StarPieceAlmsStorage::initializeData() {
    for (s32 idx = 0; idx < 16; idx++) {
        mValueArray[idx] = 0;
    }
}

