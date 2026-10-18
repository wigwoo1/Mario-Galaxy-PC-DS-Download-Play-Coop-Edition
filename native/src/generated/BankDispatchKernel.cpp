#include <Native/HeapPlacement.hpp>
#include <cstring>
#include "JSystem/JAudio2/JASBNKParser.hpp"
#include "JSystem/JAudio2/JASBasicBank.hpp"
#include "JSystem/JAudio2/JASBasicInst.hpp"
#include "JSystem/JAudio2/JASCalc.hpp"
#include "JSystem/JAudio2/JASChannel.hpp"
#include "JSystem/JAudio2/JASDrumSet.hpp"
#include "JSystem/JAudio2/JASGadget.hpp"
#include "JSystem/JAudio2/JASHeapCtrl.hpp"
#include "JSystem/JAudio2/JASInstRand.hpp"
#include "JSystem/JAudio2/JASInstSense.hpp"
#include "JSystem/JKernel/JKRSolidHeap.hpp"
#include "JSystem/JSupport/JSupport.hpp"

static u16 nativeBankU16(const void* p){const auto* b=static_cast<const u8*>(p);return (u16(b[0])<<8)|b[1];}
static float nativeBankFloat(const void* p){u32 bits=nativeBNKU32(p);float value;std::memcpy(&value,&bits,sizeof(value));return value;}
JASBank* JASBNKParser::createBank(void const* stream, JKRHeap* heap) {
    if (!stream) return nullptr;
    return createBasicBank(stream, heap);
}

u32 JASBNKParser::sUsedHeapSize;

JASBasicBank* JASBNKParser::createBasicBank(void const* stream, JKRHeap* heap) {
    if (heap == nullptr) {
        heap = JASDram;
    }

    u32 free_size = heap->getFreeSize();

    TFileHeader* filep = (TFileHeader*)stream;
    JASBasicBank* bank = nullptr;
    switch (nativeBNKU32(&filep->mVersion)) {
    case 0:
        bank = Ver0::createBasicBank(stream, heap);
        break;
    case 1:
        bank = Ver1::createBasicBank(stream, heap);
        break;
    }

    sUsedHeapSize += free_size - heap->getFreeSize();
    return bank;
}

JASBasicBank* JASBNKParser::Ver0::createBasicBank(void const* stream, JKRHeap* heap) {
    if (heap == nullptr) {
        heap = JASDram;
    }

    THeader const* header = (THeader*)stream;
    JASBasicBank* bank = new (heap, 0) JASBasicBank();
    if (bank == nullptr) {
        return nullptr;
    }

    for (int i = 0; i < 0x80; i++) {
        TInst* tinst = header->mOffsets.mInstOffset[i].ptr(header);
        if (tinst != nullptr) {
            JASBasicInst* instp = new (heap, 0) JASBasicInst();
            instp->setVolume(nativeBankFloat(&tinst->mVolume));
            instp->setPitch(nativeBankFloat(&tinst->mPitch));

            int osc_idx = 0;
            for (int j = 0; j < 2; j++) {
                TOsc* tosc = tinst->mOscOffset[j].ptr(header);
                if (tosc != nullptr) {
                    JASOscillator::Data* osc = findOscPtr(bank, header, tosc);

                    if (osc != nullptr) {
                        instp->setOsc(osc_idx, osc);
                    } else {
                        osc = new (heap, 0) JASOscillator::Data();
                        osc->mTarget = tosc->mTarget;
                        osc->_04 = nativeBankFloat(&tosc->field_0x4);

                        JASOscillator::Point* points = tosc->mPointOffset.ptr(header);
                        if (points != nullptr) {
                            const JASOscillator::Point* endPtr = getOscTableEndPtr(points);
                            int size = endPtr - points;
                            JASOscillator::Point* table = new (heap, 0) JASOscillator::Point[size];
                            JASCalc::bcopy(points, table, size * sizeof(JASOscillator::Point));
                            for(int k=0;k<size;k++){table[k]._0=static_cast<s16>(nativeBankU16(&points[k]._0));table[k]._2=static_cast<s16>(nativeBankU16(&points[k]._2));table[k]._4=static_cast<s16>(nativeBankU16(&points[k]._4));}
                            osc->mTable = table;
                        } else {
                            osc->mTable = nullptr;
                        }

                        points = tosc->field_0xc.ptr(header);
                        if (points != nullptr) {
                            const JASOscillator::Point* endPtr = getOscTableEndPtr(points);
                            int size = endPtr - points;
                            JASOscillator::Point* table = new (heap, 0) JASOscillator::Point[size];
                            JASCalc::bcopy(points, table, size * sizeof(JASOscillator::Point));
                            for(int k=0;k<size;k++){table[k]._0=static_cast<s16>(nativeBankU16(&points[k]._0));table[k]._2=static_cast<s16>(nativeBankU16(&points[k]._2));table[k]._4=static_cast<s16>(nativeBankU16(&points[k]._4));}
                            osc->rel_table = table;
                        } else {
                            osc->rel_table = nullptr;
                        }

                        osc->mScale = nativeBankFloat(&tosc->mScale);
                        osc->_14 = nativeBankFloat(&tosc->field_0x14);
                        instp->setOsc(osc_idx, osc);
                    }

                    osc_idx++;
                }
            }

            for (int j = 0; j < 2; j++) {
                TRandEffect* randEffect = tinst->mRandOffset[j].ptr(header);
                if (randEffect != nullptr) {
                    JASInstRand* rand = new (heap, 0) JASInstRand();
                    rand->mType = randEffect->mType;
                    rand->mBaseValue = nativeBankFloat(&randEffect->mBaseValue);
                    rand->mRange = nativeBankFloat(&randEffect->mRange);
                    instp->appendEffect(new (heap, 0) JASPtrListAdaptor< JASInstEffect >(rand));
                }
            }

            for (int j = 0; j < 2; j++) {
                TSenseEffect* senseEffect = tinst->mSenseOffset[j].ptr(header);
                if (senseEffect != nullptr) {
                    JASInstSense* sense = new (heap, 0) JASInstSense();
                    sense->mType = senseEffect->mType;
                    sense->setParams(senseEffect->mSenseType, senseEffect->mMaxPoint, nativeBankFloat(&senseEffect->mStartLvl), nativeBankFloat(&senseEffect->mEndLvl));
                    instp->appendEffect(new (heap, 0) JASPtrListAdaptor< JASInstEffect >(sense));
                }
            }

            instp->setKeyRegionCount(nativeBNKU32(&tinst->mKeyRegionCount), heap);
            for (int j = 0; j < nativeBNKU32(&tinst->mKeyRegionCount); j++) {
                JASBasicInst::TKeymap* keymap = instp->getKeyRegion(j);
                TKeymap* tkeymap = tinst->mKeymapOffset[j].ptr(header);
                keymap->setHighKey(tkeymap->mHighKey);

                keymap->setVeloRegionCount(nativeBNKU32(&tkeymap->mVeloRegionCount), heap);
                for (int k = 0; k < nativeBNKU32(&tkeymap->mVeloRegionCount); k++) {
                    JASBasicInst::TVelomap* velomap = keymap->getVeloRegion(k);
                    TVmap* tvmap = tkeymap->mVmapOffset[k].ptr(header);

                    velomap->_0 = tvmap->_0;
                    velomap->_4 = nativeBNKU32(&tvmap->_4) & 0xFFFF;
                    velomap->mVolume = nativeBankFloat(&tvmap->mVolume);
                    velomap->mPitch = nativeBankFloat(&tvmap->mPitch);
                }
            }

            bank->setInst(i, instp);
        }
    }

    for (int i = 0; i < 12; i++) {
        TPerc* tperc = header->mOffsets.mPercOffset[i].ptr(header);
        if (tperc != nullptr) {
            JASDrumSet* setp = new (heap, 0) JASDrumSet();

            for (int j = 0; j < 0x80; j++) {
                TPmap* tpmap = tperc->mPmapOffset[j].ptr(header);
                if (tpmap != nullptr) {
                    JASDrumSet::TPerc* percp = new (heap, 0) JASDrumSet::TPerc();
                    percp->setVolume(nativeBankFloat(&tpmap->mVolume));
                    percp->setPitch(nativeBankFloat(&tpmap->mPitch));
                    if (nativeBNKU32(&tperc->mMagic) == 'PER2') {
                        percp->setPan(tperc->mPan[j] / 127.0f);
                        percp->setRelease(nativeBankU16(&tperc->mRelease[j]));
                    }

                    for (int k = 0; k < 2; k++) {
                        TRandEffect* randEffect = tpmap->mRandOffset[k].ptr(header);
                        if (randEffect != nullptr) {
                            JASInstRand* rand = new (heap, 0) JASInstRand();
                            rand->mType = randEffect->mType;
                            rand->mBaseValue = nativeBankFloat(&randEffect->mBaseValue);
                            rand->mRange = nativeBankFloat(&randEffect->mRange);
                            percp->appendEffect(new (heap, 0) JASPtrListAdaptor< JASInstEffect >(rand));
                        }
                    }

                    percp->setVeloRegionCount(nativeBNKU32(&tpmap->mVeloRegionCount), heap);
                    for (int k = 0; k < nativeBNKU32(&tpmap->mVeloRegionCount); k++) {
                        JASDrumSet::TVelomap* velomap = percp->getVeloRegion(k);
                        TVmap* tvmap = tpmap->mVmapOffset[k].ptr(header);

                        velomap->_0 = tvmap->_0;
                        velomap->_4 = nativeBNKU32(&tvmap->_4) & 0xFFFF;
                        velomap->mVolume = nativeBankFloat(&tvmap->mVolume);
                        velomap->mPitch = nativeBankFloat(&tvmap->mPitch);
                    }

                    setp->setPerc(j, percp);
                }
            }

            bank->setInst(i + 0xe4, setp);
        }
    }

    return bank;
}

JASOscillator::Data* JASBNKParser::Ver0::findOscPtr(JASBasicBank* bank, THeader const* header, TOsc const* osc) {
    TOffsetData const* data = &header->mOffsets;
    for (int i = 0; i < 0x80; i++) {
        TInst* tinst = data->mInstOffset[i].ptr(header);
        if (tinst != nullptr) {
            for (int j = 0; j < 2; j++) {
                if (tinst->mOscOffset[j].ptr(header) == osc) {
                    JASInst* inst = bank->getInst(i);
                    if (inst != nullptr) {
                        JASInstParam param;
                        inst->getParam(0x3c, 0x7f, &param);
                        if (j < param.mNumOscillators) {
                            return param.mOscillatorData[j];
                        }
                    }
                }
            }
        }
    }
    return nullptr;
}

JASOscillator::Point const* JASBNKParser::Ver0::getOscTableEndPtr(JASOscillator::Point const* points) {
    const JASOscillator::Point* ptr = points;
    while (true) {
        s16 tmp = static_cast<s16>(nativeBankU16(&ptr->_0));
        ptr++;
        if (tmp > 10) {
            break;
        }
    }
    return ptr;
}

