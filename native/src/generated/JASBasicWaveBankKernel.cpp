#include <Native/HeapPlacement.hpp>
#include "JSystem/JAudio2/JASBasicWaveBank.hpp"
#include "JSystem/JAudio2/JASCalc.hpp"
#include "JSystem/JAudio2/JASMutex.hpp"
#include "JSystem/JKernel/JKRHeap.hpp"
#include <stdint.h>

JASBasicWaveBank::JASBasicWaveBank() {
    mWaveTable = nullptr;
    mHandleCount = 0;
    mWaveGroupArray = nullptr;
    mGroupCount = 0;
    OSInitMutex(&mMutex);
}

JASBasicWaveBank::~JASBasicWaveBank() {
    if(mWaveTable)JKRHeap::free(mWaveTable,nullptr);mWaveTable=nullptr;mHandleCount=0;
    for (int i = 0; i < mGroupCount; i++) {
        if(mWaveGroupArray[i]){mWaveGroupArray[i]->~TWaveGroup();JKRHeap::free(mWaveGroupArray[i],nullptr);}
    }

    if(mWaveGroupArray)JKRHeap::free(mWaveGroupArray,nullptr);mWaveGroupArray=nullptr;mGroupCount=0;
}

JASBasicWaveBank::TWaveGroup* JASBasicWaveBank::getWaveGroup(u32 param_0) {
    if (param_0 >= mGroupCount) {
        return nullptr;
    }

    return mWaveGroupArray[param_0];
}

void JASBasicWaveBank::setGroupCount(u32 param_0, JKRHeap* heap) {
    for (int i = 0; i < mGroupCount; i++) {
        if(mWaveGroupArray[i]){mWaveGroupArray[i]->~TWaveGroup();JKRHeap::free(mWaveGroupArray[i],nullptr);}
    }

    if(mWaveGroupArray)JKRHeap::free(mWaveGroupArray,nullptr);mWaveGroupArray=nullptr;mGroupCount=0;
    mGroupCount = 0;
    if(param_0>std::numeric_limits<u32>::max()/sizeof(TWaveGroup*))throw std::bad_alloc();
    mWaveGroupArray=param_0?new(heap,0)TWaveGroup*[param_0]():nullptr;
    for (u32 i = 0; i < param_0; i++) {
        mWaveGroupArray[i] = new (heap, 0) TWaveGroup(this);
        ++mGroupCount;
    }
}

void JASBasicWaveBank::setWaveTableSize(u32 count, JKRHeap* heap) {
    if(mWaveTable)JKRHeap::free(mWaveTable,nullptr);mWaveTable=nullptr;mHandleCount=0;
    mWaveTable = new (heap, 0) TWaveInfo*[count];
    JASCalc::bzero(mWaveTable, sizeof(TWaveHandle*) * count);
    mHandleCount = count;
}

void JASBasicWaveBank::incWaveTable(JASBasicWaveBank::TWaveGroup const* pWaveGroup) {
    JASMutexLock lock(&mMutex);
    TWaveInfo** table;
    TWaveInfo* info;
    u32 id;
    for (u32 i = 0; i < pWaveGroup->getWaveCount(); i++) {
        table = mWaveTable;
        info = &pWaveGroup->mCtrlWaveArray[i];

        id = info->getWaveID();
        TWaveInfo*& entry = table[id];
        info->mPrev = nullptr;
        info->mNext = entry;
        if (entry != nullptr) {
            entry->mPrev = info;
        }

        entry = info;
    }
}

void JASBasicWaveBank::decWaveTable(JASBasicWaveBank::TWaveGroup const* waveGroup) {
    JASMutexLock lock(&mMutex);
    for (u32 i = 0; i < waveGroup->getWaveCount(); i++) {
        TWaveInfo* info = &waveGroup->mCtrlWaveArray[i];
        u32 idx = info->mWaveId;

        for (TWaveInfo* info2 = mWaveTable[idx]; info2 != nullptr; info2 = info2->mNext) {
            if (info2 == info) {
                if (info2->mPrev == nullptr) {
                    mWaveTable[idx] = info2->mNext;
                } else {
                    info2->mPrev->mNext = info2->mNext;
                }

                if (info2->mNext != nullptr) {
                    info2->mNext->mPrev = info2->mPrev;
                }

                break;
            }
        }
    }
}

JASWaveHandle* JASBasicWaveBank::getWaveHandle(u32 waveId) const {
    if (waveId >= mHandleCount) {
        return nullptr;
    }

    if (mWaveTable[waveId] == nullptr) {
        return nullptr;
    }

    return &mWaveTable[waveId]->mWaveHandle;
}

JASBasicWaveBank::TWaveGroup::TWaveGroup(JASBasicWaveBank* pBank) {
    mBank = pBank;
    mCtrlWaveArray = nullptr;
    mWaveCount = 0;
}

JASBasicWaveBank::TWaveGroup::~TWaveGroup() {
    if(mCtrlWaveArray){for(u32 i=0;i<mWaveCount;++i)mCtrlWaveArray[i].~TWaveInfo();JKRHeap::free(mCtrlWaveArray,nullptr);}mCtrlWaveArray=nullptr;mWaveCount=0;
}

void JASBasicWaveBank::TWaveGroup::setWaveCount(u32 count, JKRHeap* heap) {
    if(mCtrlWaveArray){for(u32 i=0;i<mWaveCount;++i)mCtrlWaveArray[i].~TWaveInfo();JKRHeap::free(mCtrlWaveArray,nullptr);}mCtrlWaveArray=nullptr;mWaveCount=0;
    mWaveCount = count;
    if(count>std::numeric_limits<u32>::max()/sizeof(TWaveInfo))throw std::bad_alloc();
    mCtrlWaveArray=count?static_cast<TWaveInfo*>(JKRHeap::alloc(count*sizeof(TWaveInfo),alignof(TWaveInfo),heap)):nullptr;
    if(count&&!mCtrlWaveArray){mWaveCount=0;throw std::bad_alloc();}
    for(u32 i=0;i<count;++i)new(mCtrlWaveArray+i)TWaveInfo();

    for (int i = 0; i < count; i++) {
        mCtrlWaveArray[i].mWaveHandle.mHeap = &mHeap;
        mCtrlWaveArray[i].mWaveHandle.mWaveInfo._24 = &_48;
    }
}

void JASBasicWaveBank::TWaveGroup::setWaveInfo(int index, u32 waveId, const JASWaveInfo& waveInfo) {
    mCtrlWaveArray[index].mWaveId = (u16)waveId;
    mCtrlWaveArray[index].mWaveHandle.mWaveInfo.copy(waveInfo);
    mCtrlWaveArray[index].mWaveHandle.mWaveInfo._24 = &_48;
}

void JASBasicWaveBank::TWaveGroup::onLoadDone() {
    mBank->incWaveTable(this);
}

void JASBasicWaveBank::TWaveGroup::onEraseDone() {
    mBank->decWaveTable(this);
}

int JASBasicWaveBank::TWaveHandle::getWavePtr() const {
    void* base = mHeap->getBase();
    if (base == 0) {
        return 0;
    }

    return (intptr_t)base + mWaveInfo.mAWStartOffs;
}

