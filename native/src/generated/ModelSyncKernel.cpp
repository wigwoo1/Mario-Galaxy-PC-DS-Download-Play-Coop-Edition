#include <Game/Util/ModelUtil.hpp>
#include <JSystem/J3DGraphAnimator/J3DModel.hpp>
#include <JSystem/J3DGraphBase/J3DShape.hpp>
#include <JSystem/J3DGraphBase/J3DMaterial.hpp>
namespace MR {
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
}
