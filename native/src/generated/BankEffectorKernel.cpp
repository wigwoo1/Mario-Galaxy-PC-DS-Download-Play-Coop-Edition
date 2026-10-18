#include <Native/HeapPlacement.hpp>
#include <cstring>
#include "JSystem/JAudio2/JASBNKParser.hpp"
#include "JSystem/JAudio2/JASInstRand.hpp"
#include "JSystem/JAudio2/JASInstSense.hpp"
static float nativeBankFloat(const void* p){u32 bits=nativeBNKU32(p);float value;std::memcpy(&value,&bits,sizeof(value));return value;}
JASInstEffect* JASBNKParser::Ver1::createEffector(void const* stream, JKRHeap* heap) {
    u32* data = (u32*)(intptr_t)stream;
    switch (nativeBNKU32(data++)) {
    case 'Rand': {
        TRandData* randData = (TRandData*)data;
        JASInstRand* rand = new (heap, 0) JASInstRand();
        rand->mType = randData->mType;
        rand->setBase(nativeBankFloat(&randData->mBase));
        rand->setRange(nativeBankFloat(&randData->mRange));
        return rand;
    }
    case 'Sens': {
        TSenseData* senseData = (TSenseData*)data;
        JASInstSense* sense = new (heap, 0) JASInstSense();
        sense->mType = senseData->mType;
        sense->setParams(senseData->mSenseType, senseData->mMaxPoint, nativeBankFloat(&senseData->mStartLvl), nativeBankFloat(&senseData->mEndLvl));
        return sense;
    }
    default:
        return nullptr;
    }
}


