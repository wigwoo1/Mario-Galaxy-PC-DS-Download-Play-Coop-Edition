#include <memory>
#include <stdexcept>
#include <Native/HeapPlacement.hpp>
#include <Native/GDCurrentScope.hpp>
#include <Native/GpuResources.hpp>
#include "Game/System/ShapePacketUserData.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/SchedulerUtil.hpp"
#include "JSystem/J3DGraphAnimator/J3DModel.hpp"
#include "JSystem/J3DGraphAnimator/J3DModelData.hpp"
#include "JSystem/J3DGraphBase/J3DMaterial.hpp"
#include "JSystem/J3DGraphBase/J3DShape.hpp"
#include "JSystem/J3DGraphBase/J3DSys.hpp"
#include "JSystem/J3DGraphBase/J3DTexture.hpp"
#include "JSystem/JKernel/JKRHeap.hpp"
#include <revolution/gd.h>
#include <revolution/gd/GDTransform.h>

ShapePacketUserData::ShapePacketUserData() : mTexGenNum(), mDisplayListSize(), mDisplayList() {
}

void ShapePacketUserData::init(J3DMaterial* pMaterial) {
    if(mDisplayList)throw std::logic_error("Shape packet userdata already initialized");
    MR::ProhibitSchedulerAndInterrupts prohibit(false);
    const bool isEnvMap = MR::isUseTexMtxEnvMap(pMaterial);
    u32 size = isEnvMap ? 0x20 : 0;
    mTexGenNum = pMaterial->getTexGenNum();
    if(mTexGenNum>8)throw std::out_of_range("Shape packet texture coordinate count");
    for (u32 i = 0; i < mTexGenNum; i++) {
        J3DTexCoord* pTexCoord = pMaterial->getTexCoord(i);
        if (pTexCoord->getTexGenSrc() == GX_TG_POS) {
            mTexMtxInfo[i].mType = 1;
            mTexMtxInfo[i].mPostMtx = GX_PTTEXMTX0 + i * 3;
            size += 0x20;
        } else if (pTexCoord->getTexGenSrc() == GX_TG_NRM) {
            mTexMtxInfo[i].mType = 2;
            mTexMtxInfo[i].mPostMtx = GX_PTTEXMTX0 + i * 3;
            size += 0x20;
        } else if (isEnvMap && pTexCoord->getTexGenMtx() != GX_IDENTITY) {
            mTexMtxInfo[i].mType = 3;
            mTexMtxInfo[i].mPostMtx = GX_PTTEXMTX0 + i * 3;
            size += 0x20;
        } else {
            mTexMtxInfo[i].mType = 0;
            mTexMtxInfo[i].mPostMtx = GX_PTIDENTITY;
        }
    }

    struct HeapDelete{void operator()(u8* bytes)const{if(bytes)JKRHeap::free(bytes,JKRHeap::findFromRoot(bytes));}};
    std::unique_ptr<u8,HeapDelete> storage(new (32) u8[size+0x40]);mDisplayList=storage.get();
    struct InitGuard{ShapePacketUserData* owner;bool complete=false;~InitGuard(){if(!complete){owner->mDisplayList=nullptr;owner->mDisplayListSize=0;}}} initialization{this};
    DCInvalidateRange(mDisplayList, size + 0x40);
    Native::GDCurrentScope writerScope;
    GDLObj displayList;
    GDInitGDLObj(&displayList, mDisplayList, size + 0x40);
    GDSetCurrent(&displayList);
    if (isEnvMap) {
        GDSetCurrentMtx(0, GX_IDENTITY, GX_IDENTITY, GX_IDENTITY, GX_IDENTITY, GX_IDENTITY, GX_IDENTITY, GX_IDENTITY, GX_IDENTITY);
    }

    for (u32 i = 0; i < mTexGenNum; i++) {
        if (mTexMtxInfo[i].mType != 0) {
            J3DTexCoord* pTexCoord = pMaterial->getTexCoord(i);
            GDSetTexCoordGen(static_cast< GXTexCoordID >(i), static_cast< GXTexGenType >(pTexCoord->getTexGenType()),
                             static_cast< GXTexGenSrc >(pTexCoord->getTexGenSrc()), GX_FALSE, mTexMtxInfo[i].mPostMtx);
        }
    }

    GDPadCurr32();
    mDisplayListSize = GDGetGDLObjOffset(&displayList);
    DCStoreRange(mDisplayList, size + 0x40);
    Native::gpuResources().add(mDisplayList,size+0x40);storage.release();initialization.complete=true;
}

void ShapePacketUserData::callDL() const {
    GXCallDisplayList(mDisplayList, mDisplayListSize);
}

void ShapePacketUserData::loadTexMtx(J3DMaterial* pMaterial, int slot, u16 index) const {
    TPos3f mtx;
    for (u32 i = 0; i < mTexGenNum; i++) {
        if (mTexMtxInfo[i].mType != 0) {
            if (mTexMtxInfo[i].mType == 2) {
                mtx.set(j3dSys.getModelDrawMtx(index));
                mtx.setTrans(0.0f, 0.0f, 0.0f);
                GXLoadTexMtxImm(mtx, GX_TEXMTX0 + slot * 3, static_cast< GXTexMtxType >(pMaterial->getTexCoord(i)->getTexGenType()));
            }

            J3DTexMtx* const pTexMtx = pMaterial->getTexMtx(i);
            pTexMtx->calcPostTexMtx(MR::getCameraInvViewMtx());
            GXLoadTexMtxImm(pTexMtx->getMtx(), mTexMtxInfo[i].mPostMtx, GX_MTX3x4);
        }
    }
}

namespace MR {
    ShapePacketUserData* getJ3DShapePacketUserData(const J3DShapePacket* pShapePacket) {
        if (pShapePacket->getUserArea() != nullptr) {
            return static_cast< ShapePacketUserData* >(pShapePacket->getUserArea());
        }

        return nullptr;
    }

    void initJ3DShapePacketUserData(J3DModel* pModel) {
        J3DModelData* pModelData = pModel->getModelData();
        const u16 materialNum = pModelData->getMaterialNum();
        for (u16 i = 0; i < materialNum; i++) {
            J3DMaterial* pMaterial = pModelData->getMaterialNodePointer(i);
            J3DShapePacket* pShapePacket = pModel->getShapePacket(pMaterial->getShape()->getIndex());
            if (isEnvelope(pMaterial) && (isUseTexMtxProjMap(pMaterial) || isUseTexMtxEnvMap(pMaterial))) {
                std::unique_ptr<ShapePacketUserData> ownedUserData(new ShapePacketUserData()); ShapePacketUserData* pUserData=ownedUserData.get();
                pUserData->init(pMaterial);
                if (pUserData != nullptr) {
                    pShapePacket->setUserArea(reinterpret_cast< uintptr_t >(pUserData)); ownedUserData.release();
                }
            }
        }
    }
}  // namespace MR

ShapePacketUserData::~ShapePacketUserData(){if(mDisplayList){Native::gpuResources().remove(mDisplayList);JKRHeap::free(mDisplayList,JKRHeap::findFromRoot(mDisplayList));}}

