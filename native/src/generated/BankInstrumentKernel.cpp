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


static u16 nativeBankU16(const void* p){const auto* b=static_cast<const unsigned char*>(p);return (u16(b[0])<<8)|b[1];}
static float nativeBankFloat(const void* p){u32 bits=nativeBNKU32(p);float value;std::memcpy(&value,&bits,sizeof(value));return value;}
JASBasicBank* JASBNKParser::Ver1::createBasicBank(void const* stream, JKRHeap* heap) {
    if (heap == nullptr) {
        heap = JASDram;
    }

    JASBasicBank* bank = new (heap, 0) JASBasicBank();
    if (bank == nullptr) {
        return nullptr;
    }

    TEnvtChunk* envt_chunk = (TEnvtChunk*)findChunk(stream, 'ENVT');
    TOscChunk* osc_chunk = (TOscChunk*)findChunk(stream, 'OSCT');
    TListChunk* list_chunk = (TListChunk*)findChunk(stream, 'LIST');

    if (!envt_chunk || !osc_chunk || !list_chunk) return nullptr;
    u8* envt = new (heap, 2) u8[nativeBNKU32(&envt_chunk->mSize)];
    JASCalc::bcopy(envt_chunk->mData, envt, nativeBNKU32(&envt_chunk->mSize));
    for(u32 j=0;j+2<=nativeBNKU32(&envt_chunk->mSize);j+=2){u16 value=nativeBankU16(envt+j);std::memcpy(envt+j,&value,2);}

    u32* ptr = &osc_chunk->mCount;
    u32 count = nativeBNKU32(ptr++);
    JASOscillator::Data* osc_data = new (heap, 0) JASOscillator::Data[count];
    for (int i = 0; i < count; i++, ptr += sizeof(TOsc) >> 2) {
        TOsc* op = (TOsc*)ptr;
        JASOscillator::Data* data = &osc_data[i];
        data->mTarget = op->mTarget;
        data->_04 = nativeBankFloat(&op->_08);
        data->mScale = nativeBankFloat(&op->mScale);
        data->_14 = nativeBankFloat(&op->_18);
        data->mTable = (JASOscillator::Point*)(envt + nativeBNKU32(&op->mTableOffset));
        data->rel_table = (JASOscillator::Point*)(envt + nativeBNKU32(&op->_10));
    }

    TListChunk* list = list_chunk;
    for (int i = 0; i < nativeBNKU32(&list->count); i++) {
        if (nativeBNKU32(&list->mOffsets[i]) != 0) {
            u32* data = (u32*)((intptr_t)stream + nativeBNKU32(&list->mOffsets[i]));
            switch (nativeBNKU32(data++)) {
            case 'Inst': {
                JASBasicInst* instp = new (heap, 0) JASBasicInst();
                u32 oscCount = nativeBNKU32(data++);
                for (int j = 0; j < oscCount; j++) {
                    u32 index = nativeBNKU32(data++);
                    instp->setOsc(j, &osc_data[index]);
                }

                u32 effectCount = nativeBNKU32(data++);
                for (int j = 0; j < effectCount; j++) {
                    JASInstEffect* effect = createEffector((void*)((intptr_t)stream + nativeBNKU32(data++)), heap);
                    instp->appendEffect(new (heap, 0) JASPtrListAdaptor< JASInstEffect >(effect));
                }

                u32 keyRegionCount = nativeBNKU32(data++);
                instp->setKeyRegionCount(keyRegionCount, heap);
                for (int j = 0; j < keyRegionCount; j++) {
                    JASBasicInst::TKeymap* keymap = instp->getKeyRegion(j);
                    keymap->setHighKey(nativeBNKU32(data) >> 0x18);
                    u32 veloCount = nativeBNKU32(data+1);
                    data += 2;
                    keymap->setVeloRegionCount(veloCount, heap);
                    for (int k = 0; k < veloCount; k++) {
                        JASBasicInst::TVelomap* velomap = keymap->getVeloRegion(k);
                        TVeloData* veloData = (TVeloData*)data;
                        velomap->_0 = veloData->_0;
                        velomap->_4 = (u16)nativeBNKU32(&veloData->_4);
                        velomap->mVolume = nativeBankFloat(&veloData->mVolume);
                        velomap->mPitch = nativeBankFloat(&veloData->mPitch);
                        data += 4;
                    }
                }
                TInstData* instData = (TInstData*)data;
                instp->setVolume(nativeBankFloat(&instData->mVolume));
                instp->setPitch(nativeBankFloat(&instData->mPitch));
                bank->setInst(i, instp);
                break;
            }

            case 'Perc': {
                JASDrumSet* drump = new (heap, 0) JASDrumSet();
                u32 pmap_count = nativeBNKU32(data+1);
                u32 count = nativeBNKU32(data++);
                for (int j = 0; j < count; j++) {
                    u32 offset = nativeBNKU32(data++);
                    if (offset != 0) {
                        JASDrumSet::TPerc* percp = new (heap, 0) JASDrumSet::TPerc();
                        u32* ptr = (u32*)((intptr_t)stream + offset);
                        TPercData* perc_data = (TPercData*)(ptr + 1);
                        percp->setVolume(nativeBankFloat(&perc_data->mVolume));
                        percp->setPitch(nativeBankFloat(&perc_data->mPitch));
                        percp->setPan((f32)perc_data->mPan / 127.0f);
                        percp->setRelease(nativeBankU16(&perc_data->mRelease));

                        ptr += 4;
                        u32 effectCount = nativeBNKU32(ptr++);
                        for (u32 j = 0; j < effectCount; j++) {
                            JASInstEffect* effect = createEffector((void*)((intptr_t)stream + nativeBNKU32(ptr++)), heap);
                            percp->appendEffect(new (heap, 0) JASPtrListAdaptor< JASInstEffect >(effect));
                        }

                        u32 veloCount = nativeBNKU32(ptr++);
                        percp->setVeloRegionCount(veloCount, heap);
                        for (int k = 0; k < veloCount; k++) {
                            JASDrumSet::TVelomap* velomap = percp->getVeloRegion(k);
                            TVeloData* veloData = (TVeloData*)ptr;
                            velomap->_0 = veloData->_0;
                            velomap->_4 = (u16)nativeBNKU32(&veloData->_4);
                            velomap->mVolume = nativeBankFloat(&veloData->mVolume);
                            velomap->mPitch = nativeBankFloat(&veloData->mPitch);
                            ptr += 4;
                        }
                        drump->setPerc(j, percp);
                    }
                }
                bank->setInst(i, drump);
                break;
            }
            }
        }
    }

    return bank;
}


