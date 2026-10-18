#pragma once
#include "native_model_resource.hpp"
namespace NativeModelInit {
inline bool isEnvelope(J3DMaterial* pMaterial) {
        return pMaterial->getShape()->mHasPNMTXIdx;
    }
inline bool isUseTexMtx(J3DMaterial* pMaterial, u32 idx) {
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
inline bool isUseTexMtxEnvMap(J3DMaterial* pMaterial) {
        for (u32 idx = 0; idx < 8; idx++) {
            J3DTexMtx* texMtx = pMaterial->mTexGenBlock->getTexMtx(idx);
            if (texMtx == nullptr) {
                continue;
            }

            u32 temp = texMtx->getTexMtxInfo().mInfo & 0x3F;
            if (temp == 1 || temp == 6 || temp == 7) {
                if (!isUseTexMtx(pMaterial, idx)) {
                    return false;
                }

                return true;
            }
        }

        return false;
    }
inline bool isUseTexMtxProjMap(J3DMaterial* pMaterial) {
        for (u32 idx = 0; idx < 8; idx++) {
            J3DTexMtx* texMtx = pMaterial->mTexGenBlock->getTexMtx(idx);
            if (texMtx == nullptr) {
                continue;
            }

            u32 temp = texMtx->getTexMtxInfo().mInfo & 0x3F;
            if (temp == 2 || temp == 3 || temp == 8 || temp == 9) {
                return true;
            }
        }

        return false;
    }
inline void initEnvelopeAndEnvMapOrProjMapModelData(J3DModelData* pModelData, NativeModelResource& resources, JKRHeap& heap) {
        bool doSort = false;

        for (int i = 0; i < pModelData->getMaterialNum(); i++) {
            J3DMaterial* material = pModelData->getMaterialNodePointer(i);
            if (!isEnvelope(material)) {
                continue;
            }

            bool isUseEnvMap = isUseTexMtxEnvMap(material);
            bool isUseProjMap = isUseTexMtxProjMap(material);
            if (!isUseEnvMap && !isUseProjMap) {
                continue;
            }

            J3DShape* shape = material->getShape();
            resources.shapes->makeCommandPrivate(*shape, heap);
            doSort = true;
            if (isUseEnvMap) {
                shape->setTexMtxLoadType(0x2000);
            }

            for (u32 idx = 0; idx < 8; idx++) {
                J3DTexCoord* texCoord = material->getTexCoord(idx);
                GXAttr attr = static_cast< GXAttr >(idx + 1);
                if (texCoord->getTexGenSrc() == 1) {
                    shape->addTexMtxIndexInDL(attr, 30);
                    shape->addTexMtxIndexInVcd(attr); resources.shapes->retainRewrittenGeometry(); resources.shapes->retainRewrittenDescriptors();
                } else if (texCoord->getTexGenSrc() == 0) {
                    shape->addTexMtxIndexInDL(attr, 0);
                    shape->addTexMtxIndexInVcd(attr); resources.shapes->retainRewrittenGeometry(); resources.shapes->retainRewrittenDescriptors();
                }
            }
        }

        resources.shapes->prepareVertexCommands(pModelData->getVertexData());
        if (doSort) {
            pModelData->mShapeTable.sortVcdVatCmd();
        }
    }
}

