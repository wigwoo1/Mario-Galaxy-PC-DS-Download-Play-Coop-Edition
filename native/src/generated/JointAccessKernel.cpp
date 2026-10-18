#include "Game/Util/JointUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include <JSystem/J3DGraphAnimator/J3DJoint.hpp>
#include <JSystem/J3DGraphAnimator/J3DModel.hpp>
#include <JSystem/J3DGraphAnimator/J3DModelData.hpp>
#include <JSystem/J3DGraphAnimator/J3DMtxBuffer.hpp>
#include <JSystem/J3DGraphBase/J3DMaterial.hpp>
#include <JSystem/J3DGraphBase/J3DShape.hpp>
#include <JSystem/JGeometry.hpp>
#include <JSystem/JUtility/JUTNameTab.hpp>

namespace MR {
    J3DJoint* getJoint(J3DModel* pModel, const char* pName) {
        return pModel->mModelData->mJointTree.mJointNodePointer[(u16)pModel->mModelData->mJointTree.mJointName->getIndex(pName)];
    }

    J3DJoint* getJoint(const LiveActor* pActor, const char* pName) {
        return MR::getJoint(MR::getJ3DModel(pActor), pName);
    }

    J3DJoint* getJoint(const LiveActor* pActor, u16 idx) {
        return MR::getJ3DModelData(pActor)->mJointTree.mJointNodePointer[idx];
    }

    MtxPtr getJointMtx(J3DModel* pModel, const char* pName) {
        s32 idx = pModel->mModelData->mJointTree.mJointName->getIndex(pName);
        return pModel->mMtxBuffer->mpAnmMtx[idx];
    }

    MtxPtr getJointMtx(const LiveActor* pActor, const char* pName) {
        return getJointMtx(pActor, getJointIndex(pActor, pName));
    }

    MtxPtr getJointMtx(const LiveActor* pActor, int idx) {
        return MR::getJ3DModel(pActor)->mMtxBuffer->mpAnmMtx[idx];
    }

    u16 getJointIndex(const LiveActor* pActor, const char* pName) NO_INLINE {
        return MR::getJ3DModelData(pActor)->mJointTree.mJointName->getIndex(pName);
    }

    const char* getJointName(const LiveActor* pActor, int idx) {
        return MR::getJ3DModelData(pActor)->mJointTree.mJointName->getName(idx);
    }

    s32 getJointNum(const LiveActor* pActor) {
        return MR::getJ3DModelData(pActor)->mJointTree.mJointNum;
    }

    bool isExistJoint(const LiveActor* pActor, const char* pName) {
        return MR::getJ3DModelData(pActor)->mJointTree.mJointName->getIndex(pName) != -1;
    }

    void copyJointPos(J3DModel* pModel, const char* pName, TVec3f* pPos) {
        MtxPtr jointMtx = MR::getJointMtx(pModel, pName);
        pPos->set< f32 >(jointMtx[0][3], jointMtx[1][3], jointMtx[2][3]);
    }

    void copyJointPos(const LiveActor* pActor, const char* pName, TVec3f* pPos) {
        copyJointPos(MR::getJ3DModel(pActor), pName, pPos);
    }

    void copyJointPos(J3DModel* pModel, int idx, TVec3f* pPos) {
        MtxPtr jointMtx = pModel->mMtxBuffer->mpAnmMtx[idx];
        pPos->set< f32 >(jointMtx[0][3], jointMtx[1][3], jointMtx[2][3]);
    }

    void copyJointPos(const LiveActor* pActor, int idx, TVec3f* pPos) {
        copyJointPos(MR::getJ3DModel(pActor), idx, pPos);
    }

    f32 getJointTransX(const J3DJoint* pJoint) {
        return pJoint->mTransformInfo.mTranslate.x;
    }

    f32 getJointTransY(const J3DJoint* pJoint) {
        return pJoint->mTransformInfo.mTranslate.y;
    }

    f32 getJointTransZ(const J3DJoint* pJoint) {
        return pJoint->mTransformInfo.mTranslate.z;
    }

    J3DJoint* searchChildJoint(J3DJoint* pRoot, J3DJoint* pJoint) {
        if (pRoot == nullptr) {
            return nullptr;
        }

        if (pRoot == pJoint) {
            return reinterpret_cast< J3DJoint* >(-1);
        }

        J3DJoint* pChild = pRoot->mChild;
        if (pChild == pJoint) {
            return pRoot;
        }

        while (pChild != nullptr) {
            pChild = pChild->mYounger;
            if (pChild == pJoint) {
                return pRoot;
            }
        }

        J3DJoint* pParent = searchChildJoint(pRoot->mYounger, pJoint);
        if (pParent != nullptr) {
            return pParent;
        }

        return searchChildJoint(pRoot->mChild, pJoint);
    }

    J3DJoint* getParentJoint(J3DModelData* pModelData, J3DJoint* pJoint) {
        J3DJoint* pParent = searchChildJoint(pModelData->mJointTree.mJointNodePointer[0], pJoint);
        if (pParent == reinterpret_cast< J3DJoint* >(-1)) {
            return nullptr;
        }

        return pParent;
    }

    J3DJoint* getParentJoint(const LiveActor* pActor, J3DJoint* pJoint) {
        return getParentJoint(getJ3DModelData(pActor), pJoint);
    }
}
