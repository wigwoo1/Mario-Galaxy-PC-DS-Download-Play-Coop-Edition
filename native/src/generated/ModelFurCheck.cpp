#include <JSystem/J3DGraphAnimator/J3DModelData.hpp>
#include <JSystem/JUtility/JUTNameTab.hpp>
#include <JSystem/J3DGraphAnimator/J3DJoint.hpp>
#include <cstring>
namespace MR {
    bool isUseFur(const J3DModelData* pModelData) {
        for (u16 i = 0; i < pModelData->getMaterialNum(); i++) {
            if (strstr(pModelData->getMaterialName()->getName(i), "Fur")) {
                return true;
            }
        }

        return false;
    }
    void invalidateMtxCalc(J3DModelData* pModelData) {
        u16 jointNum = pModelData->getJointNum();
        for (u16 i = 0; i < jointNum; i++) {
            pModelData->getJointNodePointer(i)->setMtxCalc(nullptr);
        }
    }
    void invalidateJointCallback(J3DModelData* pModelData) {
        u16 jointNum = pModelData->getJointNum();
        for (u16 i = 0; i < jointNum; i++) {
            pModelData->getJointNodePointer(i)->setCallBack(nullptr);
        }
    }
}

