#include "Game/Util.hpp"
#include "Game/Player/J3DModelX.hpp"
#include "Game/System/ResourceHolder.hpp"
#include <JSystem/J3DGraphAnimator/J3DJoint.hpp>
#include <cstdio>
namespace MR {
    bool isExistCollisionResource(const LiveActor* pActor, const char* pName) {
        char buff[0x80];
        snprintf(buff, sizeof(buff), "%s.kcl", pName);
        return getResourceHolder(pActor)->mFileInfoTable->findFileInfo(buff) != nullptr;
    }
    const char* getTextureName(const LiveActor* pActor, int idx) {
        return getJ3DModelData(pActor)->getTextureName()->getName(idx);
    }
    ResTIMG* getTexture(ResourceHolder* pResourceHolder, const char* pTextureName) {
        return static_cast< ResTIMG* >(pResourceHolder->mFileInfoTable->getRes(pTextureName));
    }
    s16 getBrkFrameMax(const LiveActor* pActor, const char* pBrkName) {
        return static_cast< J3DAnmBase* >(getResourceHolder(pActor)->mBrkResTable->getRes(pBrkName))->mFrameMax;
    }
    ResTIMG* getResTIMG(const LiveActor* pActor, int idx) {
        return getResTIMG(getJ3DModelData(pActor), idx);
    }
    ResTIMG* getResTIMG(const J3DModelData* pModelData, int idx) {
        return pModelData->mMaterialTable.getTexture()->getResTIMG(idx);
    }
    int findPosNrmMtxIndexFromModel(J3DModelData* pModelData, u16 a2) {
        u16 jointNum = pModelData->getJointNum();

        for (u16 i = 0; i < jointNum; i++) {
            J3DMaterial* mesh = pModelData->getJointNodePointer(i)->getMesh();
            while (mesh != nullptr) {
                int idx = findPosNrmMtxIndexFromShape(mesh->getShape(), a2);
                if (idx != -1) {
                    return idx;
                }

                mesh = mesh->mNext;
            }
        }

        return -1;
    }
    void syncJointAnimation(LiveActor* pActor1, const LiveActor* pActor2) {
        getJ3DModel(pActor1)->mMtxBuffer = getJ3DModel(pActor2)->getMtxBuffer();
    }
    J3DModelData* getJ3DModelData(const char* pName) {
        char buff[0x100];
        snprintf(buff, sizeof(buff), "%s.arc", pName);
        return static_cast< J3DModelData* >(createAndAddResourceHolder(buff)->mModelResTable->getRes(pName));
    }
    bool findBckNameStringInResource(const char** pOut, const ResourceHolder* pHolder, const char* pNameString) {
        for (int i = 0; i < pHolder->mMotionResTable->mCount; i++) {
            if (isEqualStringCase(pNameString, pHolder->getMotionName(i))) {
                *pOut = pHolder->getMotionName(i);
                return true;
            }
        }

        return false;
    }
    void hideMaterial(J3DModel* pModel, const char* pMaterialName) {
        J3DShapePacket* pckt = pModel->getMatPacket(getMaterialNo(pModel, pMaterialName))->getShapePacket();
        pckt->mFlags |= 0x10;
    }
    void hideMaterial(const LiveActor* pActor, const char* pMaterialName) {
        hideMaterial(getJ3DModel(pActor), pMaterialName);
    }
    void showMaterial(J3DModel* pModel, const char* pMaterialName) {
        J3DShapePacket* pckt = pModel->getMatPacket(getMaterialNo(pModel, pMaterialName))->getShapePacket();
        pckt->mFlags &= ~0x10;
    }
    void showMaterial(const LiveActor* pActor, const char* pMaterialName) {
        showMaterial(getJ3DModel(pActor), pMaterialName);
    }
    s16 getBckFrameMax(const LiveActor* pActor, const char* pBckName) {
        return static_cast< J3DAnmBase* >(getResourceHolder(pActor)->mMotionResTable->getRes(pBckName))->mFrameMax;
    }
}

