#include <Native/HeapPlacement.hpp>
#include "JSystem/JAudio2/JASBasicInst.hpp"
#include "JSystem/JAudio2/JASCalc.hpp"
#include "JSystem/JAudio2/JASGadget.hpp"
#include "JSystem/JAudio2/JASInstEffect.hpp"
#include "JSystem/JKernel/JKRHeap.hpp"

JASBasicInst::JASBasicInst() : mVolume(1.0f), mPitch(1.0f), mEffects(nullptr), mKeymapCount(0), mKeymap(nullptr) {
    JASCalc::bzero(mOscillatorData, sizeof(mOscillatorData));
}

JASBasicInst::~JASBasicInst() {
    if (mKeymap) { for (int i=0;i<mKeymapCount;++i) mKeymap[i].~TKeymap(); JKRHeap::free(mKeymap, nullptr); } mKeymap=nullptr; mKeymapCount=0;
}

bool JASBasicInst::getParam(int pitch, int velocity, JASInstParam* pParams) const {
    pParams->mChannelType = JASChannel::CH_WAVE;
    pParams->_24 = false;
    pParams->mOscillatorData = (JASOscillator::Data**)&mOscillatorData;
    pParams->mNumOscillators = 2;
    pParams->mVolume = mVolume;
    pParams->mPitch = mPitch;

    for (JASPtrListAdaptor< JASInstEffect >* it = mEffects.begin(); it != nullptr; it = it->getNext()) {
        it->get()->effect(pitch, velocity, pParams);
    }

    const TKeymap* keyMap = nullptr;
    for (int i = 0; i < mKeymapCount; i++) {
        if (pitch <= mKeymap[i].mHighKey) {
            keyMap = &mKeymap[i];
            break;
        }
    }

    if (keyMap == nullptr) {
        return false;
    }

    for (int i = 0; i < keyMap->getVeloRegionCount(); i++) {
        const TVelomap* veloMap = keyMap->getVeloRegion(i);
        if (velocity <= veloMap->_0) {
            pParams->mVolume *= veloMap->mVolume;
            pParams->mPitch *= veloMap->mPitch;
            pParams->_18 = veloMap->_4;
            return true;
        }
    }

    return false;
}

void JASBasicInst::setKeyRegionCount(u32 count, JKRHeap* pHeap) {
    if (mKeymap) { for (int i=0;i<mKeymapCount;++i) mKeymap[i].~TKeymap(); JKRHeap::free(mKeymap, nullptr); } mKeymap=nullptr; mKeymapCount=0;
    if (count > std::numeric_limits<u32>::max()/sizeof(TKeymap)) throw std::bad_alloc();
    mKeymap = count ? static_cast<TKeymap*>(JKRHeap::alloc(count*sizeof(TKeymap), alignof(TKeymap), pHeap)) : nullptr;
    if (count && !mKeymap) { mKeymapCount=0; throw std::bad_alloc(); }
    for (u32 i=0;i<count;++i) new (mKeymap+i) TKeymap();
    mKeymapCount = count;
}

void JASBasicInst::appendEffect(JASPtrListAdaptor< JASInstEffect >* p) {
    if (mEffects.isEmpty()) {
        mEffects.mList = p;
        return;
    }

    JASPtrListAdaptor< JASInstEffect >* it = mEffects.begin();
    for (; it->getNext() != nullptr; it = it->getNext()) {
    }
    it->mNext = p;
}

void JASBasicInst::setOsc(int index, JASOscillator::Data const* pData) {
    mOscillatorData[index] = pData;
}

JASBasicInst::TKeymap* JASBasicInst::getKeyRegion(int index) {
    if (index >= mKeymapCount) {
        return nullptr;
    }

    return &mKeymap[index];
}

JASBasicInst::TKeymap* JASBasicInst::getKeyRegion(int index) const {
    if (index >= mKeymapCount) {
        return nullptr;
    }

    return &mKeymap[index];
}

JASBasicInst::TKeymap::~TKeymap() {
    if (mVelomap) JKRHeap::free(mVelomap, nullptr);
}

void JASBasicInst::TKeymap::setVeloRegionCount(u32 count, JKRHeap* pHeap) {
    if (mVelomap) JKRHeap::free(mVelomap, nullptr);
    mVelomap = new (pHeap, 0) TVelomap[count];
    mVelomapCount = count;
}

JASBasicInst::TVelomap* JASBasicInst::TKeymap::getVeloRegion(int index) {
    if (index >= mVelomapCount) {
        return nullptr;
    }
    return &mVelomap[index];
}

const JASBasicInst::TVelomap* JASBasicInst::TKeymap::getVeloRegion(int index) const {
    if (index >= mVelomapCount) {
        return nullptr;
    }
    return &mVelomap[index];
}

