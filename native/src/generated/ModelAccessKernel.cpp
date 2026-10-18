#include <Game/Util/FileUtil.hpp>
#include <cstdio>
#include <JSystem/J3DGraphBase/J3DShape.hpp>
#include <Game/Util/JointUtil.hpp>
#include <Game/Util/MathUtil.hpp>
#include <Game/Util/ModelUtil.hpp>
#include <Game/LiveActor/LiveActor.hpp>
#include <Game/LiveActor/ModelManager.hpp>
#include <JSystem/J3DGraphAnimator/J3DJoint.hpp>
#include <JSystem/J3DGraphAnimator/J3DModel.hpp>
#include <JSystem/J3DGraphBase/J3DMaterial.hpp>
#include <JSystem/JUtility/JUTNameTab.hpp>
#include <Game/System/ResourceHolder.hpp>
#include <Game/System/ShapePacketUserData.hpp>
#include <Game/Util/MutexHolder.hpp>
#include <Game/Util/SchedulerUtil.hpp>
#include <Game/Util/StringUtil.hpp>
#include <Game/Player/J3DModelX.hpp>
namespace MR {
bool isExistModel(const char* pName) {
        char buff[0x100];
        snprintf(buff, sizeof(buff), "/ObjectData/%s.arc", pName);
        return isFileExist(buff, false);
    }
bool isExistSubModel(const char* pModelName, const char* pSubModelName) {
        char buff[0x100];
        snprintf(buff, sizeof(buff), "%s%s", pModelName, pSubModelName);
        return isExistModel(buff);
    }
void copyJointAnimation(J3DModel* pCopyTo, J3DModel* pCopyFrom) {
        u16 jointNum = pCopyTo->getModelData()->getJointNum();
        for (u16 i = 0; i < jointNum; i++) {
            pCopyTo->setAnmMtx(i, pCopyFrom->getAnmMtx(i));
        }

        u16 wEvlpMtxNum = pCopyTo->getModelData()->getWEvlpMtxNum();
        for (u16 i = 0; i < wEvlpMtxNum; i++) {
            pCopyTo->setWeightAnmMtx(i, pCopyFrom->getWeightAnmMtx(i));
        }
    }
void copyJointAnimation(LiveActor* pCopyTo, const LiveActor* pCopyFrom) {
        return copyJointAnimation(getJ3DModel(pCopyTo), getJ3DModel(pCopyFrom));
    }
void syncMaterialAnimation(J3DModel* pModel1, J3DModel* pModel2) {
        u16 materialNum = pModel1->getModelData()->getMaterialNum();
        u16 i;
        u16 j;
        u16 idx;
        for (i = 0; i < materialNum; i++) {
            J3DShapePacket* shapePacketModel2 = pModel2->getShapePacket(i);
            idx = shapePacketModel2->getShape()->getMaterial()->getIndex();
            for (j = 0; j < materialNum; j++) {
                J3DShapePacket* shapePacketModel1 = pModel1->getShapePacket(j);
                if (shapePacketModel1->getShape()->getMaterial()->getIndex() == idx) {
                    shapePacketModel1->setDisplayListObj(shapePacketModel2->getDisplayListObj());
                    break;
                }
            }
        }
    }
void syncMaterialAnimation(LiveActor* pActor1, const LiveActor* pActor2) {
        return syncMaterialAnimation(getJ3DModel(pActor1), getJ3DModel(pActor2));
    }
void calcJ3DModel(LiveActor* pActor) {
        OSLockMutex(&MR::MutexHolder< 0 >::sMutex);
        getJ3DModel(pActor)->calc();
        OSUnlockMutex(&MR::MutexHolder< 0 >::sMutex);
    }
void calcModelBoundingBox(TBox3f* pOut, const LiveActor* pActor) {
        s32 jointNum = getJointNum(pActor);
        for (u16 i = 0; i < jointNum; i++) {
            TPos3f jointMtx;
            TBox3f bounds;
            J3DJoint* pJoint = getJoint(pActor, i);
            TVec3f jointMin(*pJoint->getMin());
            TVec3f jointMax(*pJoint->getMax());
            jointMtx.setInline(getJointMtx(pActor, i));
            TVec3f translation;
            jointMtx.getTrans(translation);
            TVec3f scale;
            jointMtx.getScale(scale);
            jointMin.mul(scale);
            jointMax.mul(scale);
            bounds.set(translation + jointMin, translation + jointMax);
            if (i == 0) {
                pOut->set(bounds.i, bounds.f);
                continue;
            }

            pOut->extend(bounds.i, bounds.f);
        }

        pOut->add(-pActor->mPosition);
    }
void calcModelBoundingRadius(f32* pOut, const LiveActor* pActor) {
        MR::getJointNum(pActor);
        TBox3f boundingBox;
        calcModelBoundingBox(&boundingBox, pActor);
        TVec3f i = boundingBox.i;
        TVec3f f = boundingBox.f;

        TVec3f max;
        max.set(MR::max(MR::abs(i.x), MR::abs(f.x)), MR::max(MR::abs(i.y), MR::abs(f.y)), MR::max(MR::abs(i.z), MR::abs(f.z)));

        *pOut = max.length();
    }
J3DModel* newJ3DModel(const ResourceHolder* pResourceHolder, const char* pChar, J3DMdlFlag mdlFlag) {
        J3DModelData* modelData = static_cast< J3DModelData* >(pResourceHolder->mModelResTable->getRes(pChar));
        J3DModel* newModel;
        invalidateMtxCalc(modelData);
        invalidateJointCallback(modelData);

        for (u16 i = 0; i < modelData->getShapeNum(); i++) {
            modelData->getShapeNodePointer(i)->mFlags &= ~0x1;
        }

        if (MR::isEqualString(pChar, "Mario") || MR::isEqualString(pChar, "Luigi") || MR::isEqualString(pChar, "MarioHandL") ||
            MR::isEqualString(pChar, "MarioHandR") || MR::isEqualString(pChar, "MarioFace") || MR::isEqualString(pChar, "LuigiFace") ||
            MR::isEqualString(pChar, "MarioShadow") || MR::isEqualString(pChar, "LuigiShadow") || MR::isEqualString(pChar, "IceMario") ||
            MR::isEqualString(pChar, "IceLuigi") || MR::isEqualString(pChar, "IceMarioHandL") || MR::isEqualString(pChar, "IceMarioHandR") ||
            MR::isEqualString(pChar, "InvincibleMarioHandL") || MR::isEqualString(pChar, "InvincibleMarioHandR") ||
            MR::isEqualString(pChar, "InvincibleMario") || MR::isEqualString(pChar, "InvincibleLuigi") || MR::isEqualString(pChar, "BeeMario") ||
            MR::isEqualString(pChar, "BeeLuigi") || MR::isEqualString(pChar, "HopperMario") || MR::isEqualString(pChar, "HopperLuigi") ||
            MR::isEqualString(pChar, "BoneMario") || MR::isEqualString(pChar, "BoneLuigi") || MR::isEqualString(pChar, "SearchLightCone") ||
            strstr(pChar, "FlexibleSphere") || strstr(pChar, "FlexibleSandPlanetParts") || MR::isEqualString(pChar, "ScaleDownRelayPlanet") ||
            MR::isEqualString(pChar, "GhostMario") || MR::isEqualString(pChar, "GhostLuigi") || MR::isEqualString(pChar, "Koura")) {
            OSLockMutex(&MR::MutexHolder< 0 >::sMutex);
            newModel = static_cast< J3DModel* >(new J3DModelX(modelData, mdlFlag, 1));
            OSUnlockMutex(&MR::MutexHolder< 0 >::sMutex);
        } else {
            OSLockMutex(&MR::MutexHolder< 0 >::sMutex);
            newModel = new J3DModel(modelData, mdlFlag, 1);
            OSUnlockMutex(&MR::MutexHolder< 0 >::sMutex);
        }

        initJ3DShapePacketUserData(newModel);
        Mtx mtx;
        PSMTXIdentity(mtx);
        newModel->unlock();
        OSLockMutex(&MR::MutexHolder< 0 >::sMutex);
        PSMTXCopy(mtx, j3dSys.mViewMtx);
        newModel->calc();
        newModel->calcMaterial();
        newModel->viewCalc();
        newModel->viewCalc();
        OSUnlockMutex(&MR::MutexHolder< 0 >::sMutex);
        if (modelData->getModelDataType() == 0) {
            ProhibitSchedulerAndInterrupts prohibit(false);
            OSLockMutex(&MR::MutexHolder< 0 >::sMutex);
            newModel->makeDL();
            OSUnlockMutex(&MR::MutexHolder< 0 >::sMutex);
        }

        newModel->lock();
        return newModel;
    }
J3DModel* getJ3DModel(const LiveActor* pActor) {
        if (pActor->mModelManager == nullptr) {
            return nullptr;
        }

        return pActor->mModelManager->getJ3DModel();
    }
J3DModelData* getJ3DModelData(const LiveActor* pActor) {
        if (pActor->mModelManager == nullptr) {
            return nullptr;
        }

        return pActor->mModelManager->getJ3DModelData();
    }
bool isUseTex(J3DMaterial* pMaterial, u16 a2) {
        for (u32 idx = 0; idx < 8; idx++) {
            if (pMaterial->getTexNo(idx) != a2) {
                continue;
            }

            for (u32 stage = 0; stage < pMaterial->getTevStageNum(); stage++) {
                if (pMaterial->getTevBlock()->getTevOrder(stage)->mTexMap == idx) {
                    return true;
                }
            }
        }

        return false;
    }
bool isNormalTexMtx(J3DMaterial* pMaterial) {
        for (u32 idx = 0; idx < 8; idx++) {
            J3DTexMtx* texMtx = pMaterial->mTexGenBlock->getTexMtx(idx);

            if (texMtx != nullptr && (texMtx->getTexMtxInfo().mInfo & 0x3F) != 0 && isUseTexMtx(pMaterial, idx)) {
                return false;
            }
        }

        return true;
    }
bool isUseTexMtx(J3DMaterial* pMaterial, u32 idx) {
        J3DTexMtx* texMtx = pMaterial->mTexGenBlock->getTexMtx(idx);

        if (texMtx != nullptr) {
            for (u32 stage = 0; stage < pMaterial->getTevStageNum(); stage++) {
                if (pMaterial->getTevBlock()->getTevOrder(stage)->mTexCoord == idx) {
                    return true;
                }
            }

            for (u32 stage = 0; stage < pMaterial->getIndBlock()->getIndTexStageNum(); stage++) {
                if (pMaterial->getIndBlock()->getIndTexOrder(stage)->mCoord == idx) {
                    return true;
                }
            }
        }

        return false;
    }
u16 getMaterialNo(J3DModelData* pModelData, const char* pMaterialName) {
        return pModelData->getMaterialName()->getIndex(pMaterialName);
    }

    u16 getMaterialNo(J3DModel* pModel, const char* pMaterialName) {
        JUTNameTab* materialName = pModel->getModelData()->getMaterialName();
        return materialName->getIndex(pMaterialName);
    }

    J3DMaterial* getMaterial(J3DModelData* pModelData, const char* pMaterialName) {
        return pModelData->getMaterialNodePointer(getMaterialNo(pModelData, pMaterialName));
    }

    J3DMaterial* getMaterial(J3DModelData* pModelData, int idx) {
        return pModelData->getMaterialNodePointer(idx);
    }

    J3DMaterial* getMaterial(J3DModel* pModel, int idx) {
        return pModel->mModelData->getMaterialNodePointer(idx);
    }

    J3DMaterial* getMaterial(const LiveActor* pActor, int idx) {
        return getJ3DModelData(pActor)->getMaterialNodePointer(idx);
    }

    s32 getMaterialNum(J3DModel* pModel) {
        return pModel->mModelData->getMaterialNum();
    }

    const char* getMaterialName(const J3DModelData* pModelData, int idx) {
        return pModelData->getMaterialName()->getName(idx);
    }

    }