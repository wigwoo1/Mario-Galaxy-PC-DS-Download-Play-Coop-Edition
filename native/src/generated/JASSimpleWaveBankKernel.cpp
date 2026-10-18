#include <Native/HeapPlacement.hpp>
#include "JSystem/JAudio2/JASSimpleWaveBank.hpp"
#include <stdint.h>

JASSimpleWaveBank::JASSimpleWaveBank() {
    mWaveTable = nullptr;
    mWaveTableSize = 0;
}

JASSimpleWaveBank::~JASSimpleWaveBank() {
    if(mWaveTable){for(u32 i=0;i<mWaveTableSize;++i)mWaveTable[i].~TWaveHandle();JKRHeap::free(mWaveTable,nullptr);}mWaveTable=nullptr;mWaveTableSize=0;
}

void JASSimpleWaveBank::setWaveTableSize(u32 size, JKRHeap* heap) {
    if(mWaveTable){for(u32 i=0;i<mWaveTableSize;++i)mWaveTable[i].~TWaveHandle();JKRHeap::free(mWaveTable,nullptr);}mWaveTable=nullptr;mWaveTableSize=0;
    if(size>std::numeric_limits<u32>::max()/sizeof(TWaveHandle))throw std::bad_alloc();
    mWaveTable=size?static_cast<TWaveHandle*>(JKRHeap::alloc(size*sizeof(TWaveHandle),alignof(TWaveHandle),heap)):nullptr;
    if(size&&!mWaveTable)throw std::bad_alloc();
    for(u32 i=0;i<size;++i)new(mWaveTable+i)TWaveHandle();
    mWaveTableSize = size;
}

JASWaveHandle* JASSimpleWaveBank::getWaveHandle(u32 no) const {
    if (no >= mWaveTableSize) {
        return nullptr;
    }
    return &mWaveTable[no];
}

void JASSimpleWaveBank::setWaveInfo(u32 no, JASWaveInfo const& waveInfo) {
    mWaveTable[no].mWaveInfo = waveInfo;
    mWaveTable[no].mWaveInfo._24 = &_48;
    mWaveTable[no].mHeap = &mHeap;
}

JASWaveArc* JASSimpleWaveBank::getWaveArc(u32 no) {
    if (no != 0) {
        return nullptr;
    }
    return this;
}

int JASSimpleWaveBank::TWaveHandle::getWavePtr() const {
    void* base = mHeap->getBase();
    if (base == nullptr) {
        return 0;
    }
    return (intptr_t)base + mWaveInfo.mAWStartOffs;
}

