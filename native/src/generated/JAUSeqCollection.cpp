#include <JSystem/JAudio2/JAUSoundTable.hpp>
#include "JSystem/JAudio2/JAUSeqCollection.hpp"

JAUSeqCollection::JAUSeqCollection() {
    mCollectionData = nullptr; mGroupNum=0; mFileSize=0; mSoundTableOffsets=nullptr;
}

void JAUSeqCollection::init(void const* param_0) {
    mCollectionData = (const JAUSeqCollectionData*)param_0;

    if (!mCollectionData || mCollectionData->magic[0] != 'S' || mCollectionData->magic[1] != 'C') {
        mCollectionData = nullptr; mGroupNum=0; mFileSize=0; mSoundTableOffsets=nullptr;
        return;
    }

    mGroupNum = nativeSoundTableU16(&mCollectionData->mGroupNum);
    mFileSize = nativeSoundTableU32(&mCollectionData->mFileSize);
    if (mFileSize < 8 || mGroupNum > (mFileSize-8)/4) {mCollectionData=nullptr;return;}
    mSoundTableOffsets = &mCollectionData->mSoundTableOffsets;
}

bool JAUSeqCollection::getSeqData(int groupId, int waveId, JAISeqData* pSeqData) {
    if (!mCollectionData || !pSeqData || groupId < 0 || waveId < 0 || groupId >= mGroupNum) {
        return false;
    }

    u32 offset = nativeSoundTableU32(reinterpret_cast<const u8*>(mSoundTableOffsets)+groupId*4);
    if (offset > mFileSize || mFileSize-offset < 4) return false;
    JAUSeqCollectionSoundTable* pSoundTable = (JAUSeqCollectionSoundTable*)((u8*)mCollectionData + offset);

    const u32 count = nativeSoundTableU32(&pSoundTable->mWaveNum);
    if (count > (mFileSize-offset-4)/4 || u32(waveId) >= count) {
        return false;
    }

    const u32 seqOffset = nativeSoundTableU32(reinterpret_cast<const u8*>(pSoundTable->mWaveSeqOffsets)+waveId*4);
    if (seqOffset >= mFileSize) return false;
    pSeqData->set((void*)mCollectionData, seqOffset);
    return true;
}

bool JAUSeqCollection::getSeqDataRegion(JAISeqDataRegion* pSeqDataRegion) {
    if (pSeqDataRegion && isValid()) {
        pSeqDataRegion->addr = (u8*)mCollectionData;
        pSeqDataRegion->size = mFileSize;
        return true;
    }

    return false;
}

JAUSeqDataMgr_SeqCollection::JAUSeqDataMgr_SeqCollection() {
    user_ = nullptr;
}

bool JAUSeqDataMgr_SeqCollection::setSeqDataUser(JAISeqDataUser* pSeqDataUser) {
    user_ = pSeqDataUser;
    return true;
}

int JAUSeqDataMgr_SeqCollection::releaseSeqData() {
    if (user_) {
        JAISeqDataRegion region;
        if (!getSeqDataRegion(&region)) return 2;
        int result = user_->releaseSeqData(region);
        return result;
    }
    return 2;
}

JAUSeqDataMgr_SeqCollection::SeqDataReturnValue JAUSeqDataMgr_SeqCollection::getSeqData(JAISoundID soundId, JAISeqData* pSeqData) {
    if (!isValid()) {
        return SeqDataReturnValue_0;
    }
    if (JAUSeqCollection::getSeqData(soundId.getGroupID(), soundId.getWaveID(), pSeqData)) {
        return SeqDataReturnValue_2;
    }
    return SeqDataReturnValue_0;
}

JAUSeqDataMgr_SeqCollection::~JAUSeqDataMgr_SeqCollection() {
}

