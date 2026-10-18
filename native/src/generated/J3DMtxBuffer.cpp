#include <Native/HeapGpuRange.hpp>
#include <Native/HeapPlacement.hpp>
#include "Native/HeapPlacement.hpp"
#include "Native/EnvelopeMatrix.hpp"
#include "JSystem/J3DGraphAnimator/J3DMtxBuffer.hpp"
#include "JSystem/J3DGraphAnimator/J3DJoint.hpp"
#include "JSystem/J3DGraphBase/J3DMaterial.hpp"
#include "JSystem/J3DGraphLoader/J3DModelLoader.hpp"
#include "JSystem/JKernel/JKRHeap.hpp"

Mtx J3DMtxBuffer::sNoUseDrawMtx;
Mtx33 J3DMtxBuffer::sNoUseNrmMtx;
Mtx* J3DMtxBuffer::sNoUseDrawMtxPtr = &J3DMtxBuffer::sNoUseDrawMtx;
Mtx33* J3DMtxBuffer::sNoUseNrmMtxPtr = &J3DMtxBuffer::sNoUseNrmMtx;

void J3DMtxBuffer::initialize() {
    mJointTree = NULL;
    mpScaleFlagArr = NULL;
    mpEvlpScaleFlagArr = NULL;
    mpAnmMtx = NULL;
    mpWeightEvlpMtx = NULL;
    mpDrawMtxArr[0] = NULL;
    mpDrawMtxArr[1] = NULL;
    mpNrmMtxArr[0] = NULL;
    mpNrmMtxArr[1] = NULL;
    mpBumpMtxArr[0] = NULL;
    mpBumpMtxArr[1] = NULL;
    mMtxNum = 1;
    mCurrentViewNo = 0;
    mpUserAnmMtx = NULL;
}

enum {
    J3DMdlDataFlag_ConcatView = 0x10,
    J3DMdlDataFlag_NoAnimation = 0x100,
};

s32 J3DMtxBuffer::create(J3DModelData* pModelData, u32 mtxNum) {
    s32 ret = kJ3DError_Success;
    mMtxNum = mtxNum;
    mJointTree = &pModelData->getJointTree();

    ret = createAnmMtx(pModelData);
    if (ret != kJ3DError_Success)
        return ret;

    ret = createWeightEnvelopeMtx(pModelData);
    if (ret != kJ3DError_Success)
        return ret;

    if (pModelData->checkFlag(J3DMdlDataFlag_NoAnimation)) {
        setNoUseDrawMtx();
    } else {
        u32 loadType = getMdlDataFlag_MtxLoadType(pModelData->getFlag());
        switch (loadType) {
        case J3DMdlDataFlag_ConcatView:
            ret = setNoUseDrawMtx();
            break;
        case 0:
        default:
            ret = createDoubleDrawMtx(pModelData, mtxNum);
            break;
        }
    }

    if (ret != kJ3DError_Success)
        return ret;

    if (pModelData->getFlag() & J3DMdlDataFlag_ConcatView) {
        pModelData->setBumpFlag(0);
    } else {
        ret = createBumpMtxArray(pModelData, mtxNum);

        if (ret != kJ3DError_Success)
            return ret;
    }

    return ret;
}

J3DError J3DMtxBuffer::createAnmMtx(J3DModelData* pModelData) {
    if (pModelData->getJointNum() != 0) {
        mpScaleFlagArr = new u8[pModelData->getJointNum()];
        mpAnmMtx = new Mtx[pModelData->getJointNum()];
        mpUserAnmMtx = mpAnmMtx;
    }

    return kJ3DError_Success;
}

s32 J3DMtxBuffer::createWeightEnvelopeMtx(J3DModelData* pModelData) {
    if (pModelData->getWEvlpMtxNum() != 0) {
        mpEvlpScaleFlagArr = new u8[pModelData->getWEvlpMtxNum()];
        mpWeightEvlpMtx = new Mtx[pModelData->getWEvlpMtxNum()];
    }

    return kJ3DError_Success;
}

s32 J3DMtxBuffer::setNoUseDrawMtx() {
    mpDrawMtxArr[0] = mpDrawMtxArr[1] = &sNoUseDrawMtxPtr;
    mpNrmMtxArr[0] = mpNrmMtxArr[1] = &sNoUseNrmMtxPtr;
    mpBumpMtxArr[0] = mpBumpMtxArr[1] = NULL;
    return kJ3DError_Success;
}

s32 J3DMtxBuffer::createDoubleDrawMtx(J3DModelData* pModelData, u32 mtxNum) {
    if (mtxNum != 0) {
        for (s32 i = 0; i < 2; i++) {
            mpDrawMtxArr[i] = new Mtx*[mtxNum]{};
            mpNrmMtxArr[i] = new Mtx33*[mtxNum]{};
            mpBumpMtxArr[i] = NULL;
        }
    }

    for (s32 i = 0; i < 2; i++) {
        for (u32 j = 0; j < mtxNum; j++) {
            if (pModelData->getDrawMtxNum() != 0) {
                mpDrawMtxArr[i][j] = new (0x20) Mtx[pModelData->getDrawMtxNum()];
                Native::bindHeapGpuNativeWords(mpDrawMtxArr[i][j],std::size_t(pModelData->getDrawMtxNum())*sizeof(Mtx));
                mpNrmMtxArr[i][j] = new (0x20) Mtx33[pModelData->getDrawMtxNum()];
                Native::bindHeapGpuNativeWords(mpNrmMtxArr[i][j],std::size_t(pModelData->getDrawMtxNum())*sizeof(Mtx33));
            }
        }
    }

    return kJ3DError_Success;
}

s32 J3DMtxBuffer::createBumpMtxArray(J3DModelData* i_modelData, u32 mtxNum) {
    if (i_modelData->getModelDataType() == 0) {
        u16 bumpMtxNum = 0;
        u16 materialCount = 0;
        u16 materialNum = i_modelData->getMaterialNum();
        for (u16 j = 0; j < materialNum; j++) {
            J3DMaterial* material = i_modelData->getMaterialNodePointer(j);
            if (material->getNBTScale()->mbHasScale == true) {
                bumpMtxNum += material->getShape()->countBumpMtxNum();
                materialCount++;
            }
        }

        if (bumpMtxNum != 0 && mtxNum != 0) {
            for (int i = 0; i < 2; i++) {
                mpBumpMtxArr[i] = new Mtx33**[(u16)materialCount]{};
            }
        }

        for (int i = 0; i < 2; i++) {
            u32 offset = 0;
            u16 materialNum = i_modelData->getMaterialNum();
            for (u16 j = 0; j < materialNum; j++) {
                J3DMaterial* material = i_modelData->getMaterialNodePointer(j);
                if (material->getNBTScale()->mbHasScale == true) {
                    mpBumpMtxArr[i][offset] = new Mtx33*[mtxNum]{};
                    material->getShape()->setBumpMtxOffset(offset);
                    offset++;
                }
            }
        }

        for (int i = 0; i < 2; i++) {
            u32 offset = 0;
            u16 materialNum = i_modelData->getMaterialNum();
            for (u16 j = 0; j < materialNum; j++) {
                J3DMaterial* material = i_modelData->getMaterialNodePointer((u16)j);
                if (material->getNBTScale()->mbHasScale == true) {
                    for (int k = 0; k < mtxNum; k++) {
                        mpBumpMtxArr[i][offset][k] = new (0x20) Mtx33[i_modelData->getDrawMtxNum()];
                if(i_modelData->getDrawMtxNum())Native::bindHeapGpuNativeWords(mpBumpMtxArr[i][offset][k],std::size_t(i_modelData->getDrawMtxNum())*sizeof(Mtx33));
                    }
                    offset++;
                }
            }
        }

        if (materialCount != 0) {
            i_modelData->setBumpFlag(1);
        }
    }

    return kJ3DError_Success;
}

static f32 J3DUnit01[] = {0.0f, 1.0f};

void J3DMtxBuffer::calcWeightEnvelopeMtx() {
    const u16* indices=mJointTree->getWEvlpMixMtxIndex();
    const f32* weights=mJointTree->getWEvlpMixWeight();
    for(unsigned i=0;i<mJointTree->getWEvlpMtxNum();++i){
        const unsigned count=mJointTree->getWEvlpMixMtxNum(i);
        mpEvlpScaleFlagArr[i]=Native::envelopeMatrix(mpAnmMtx,mJointTree->mInvJointMtx,indices,weights,mpScaleFlagArr,count,mpWeightEvlpMtx[i]);
        indices+=count;weights+=count;
    }
}

void J3DMtxBuffer::calcDrawMtx(u32 mdlFlag, Vec const& param_1, Mtx const& param_2) {
    Mtx *sp24, *sp20;
    int sp1C = 0;
    sp20 = mpAnmMtx;
    sp24 = mpWeightEvlpMtx;

    MtxPtr viewMtx;
    Mtx viewBaseMtx;
    u16 fullWgtNum;

    switch (mdlFlag) {
    case 0:
        viewMtx = j3dSys.getViewMtx();
        fullWgtNum = mJointTree->getDrawFullWgtMtxNum();
        for (u16 i = 0; i < fullWgtNum; i++) {
            PSMTXConcat(viewMtx, getAnmMtx(mJointTree->getDrawMtxIndex(i)), *getDrawMtx(i));
        }
        if (mJointTree->getDrawMtxNum() > fullWgtNum) {
            J3DPSMtxArrayConcat(viewMtx, *mpWeightEvlpMtx, *getDrawMtx(fullWgtNum), mJointTree->getWEvlpMtxNum());
        }
        return;
    case 1:
        fullWgtNum = mJointTree->getDrawFullWgtMtxNum();
        for (u16 i = 0; i < fullWgtNum; i++) {
            PSMTXCopy(getAnmMtx(mJointTree->getDrawMtxIndex(i)), *getDrawMtx(i));
        }
        fullWgtNum = mJointTree->getDrawFullWgtMtxNum();
        for (u16 i = 0; i < fullWgtNum; i++) {
            PSMTXCopy(getWeightAnmMtx(i), *getDrawMtx(mJointTree->getDrawFullWgtMtxNum() + i));
        }
        break;
    case 2:
        J3DCalcViewBaseMtx(j3dSys.getViewMtx(), param_1, param_2, viewBaseMtx);
        fullWgtNum = mJointTree->getDrawFullWgtMtxNum();
        for (u16 i = 0; i < fullWgtNum; i++) {
            PSMTXConcat(viewBaseMtx, getAnmMtx(mJointTree->getDrawMtxIndex(i)), *getDrawMtx(i));
        }
        if (mJointTree->getDrawMtxNum() > mJointTree->getDrawFullWgtMtxNum()) {
            J3DPSMtxArrayConcat(viewBaseMtx, *mpWeightEvlpMtx, *getDrawMtx(mJointTree->getDrawFullWgtMtxNum()), mJointTree->getWEvlpMtxNum());
        }
        return;
    default:
        break;
    }
}

void J3DMtxBuffer::calcNrmMtx() {
    u16 drawMtxNum = mJointTree->getDrawMtxNum();
    for (u16 i = 0; i < drawMtxNum; i++) {
        if (mJointTree->getDrawMtxFlag(i) == 0) {
            if (getScaleFlag(mJointTree->getDrawMtxIndex(i)) == 1) {
                setNrmMtx(i, *getDrawMtx(i));
            } else {
                J3DPSCalcInverseTranspose(*getDrawMtx(i), *getNrmMtx(i));
            }
        } else {
            if (getEnvScaleFlag(mJointTree->getDrawMtxIndex(i)) == 1) {
                setNrmMtx(i, *getDrawMtx(i));
            } else {
                J3DPSCalcInverseTranspose(*getDrawMtx(i), *getNrmMtx(i));
            }
        }
    }
}

void J3DMtxBuffer::calcBBoardMtx() {
    u16 drawMtxNum = mJointTree->getDrawMtxNum();
    for (u16 i = 0; i < drawMtxNum; i++) {
        if (mJointTree->getDrawMtxFlag(i) == 0) {
            u16 index = mJointTree->getDrawMtxIndex(i);
            if (mJointTree->getJointNodePointer(index)->getMtxType() == 1) {
                MtxPtr drawMtx = *getDrawMtx(i);
                J3DCalcBBoardMtx(drawMtx);

                Mtx33* nrmMtx = getNrmMtx(i);
                (*nrmMtx)[0][0] = 1.0f / drawMtx[0][0];
                (*nrmMtx)[0][1] = 0.0f;
                (*nrmMtx)[0][2] = 0.0f;
                (*nrmMtx)[1][0] = 0.0f;
                (*nrmMtx)[1][1] = 1.0f / drawMtx[1][1];
                (*nrmMtx)[1][2] = 0.0f;
                (*nrmMtx)[2][0] = 0.0f;
                (*nrmMtx)[2][1] = 0.0f;
                (*nrmMtx)[2][2] = 1.0f / drawMtx[2][2];
            } else if (mJointTree->getJointNodePointer(index)->getMtxType() == 2) {
                MtxPtr drawMtx = *getDrawMtx(i);
                J3DCalcYBBoardMtx(drawMtx);
                Mtx33* nrmMtx = getNrmMtx(i);
                J3DPSCalcInverseTranspose(drawMtx, *nrmMtx);
            }
        }
    }
}

void J3DCalcViewBaseMtx(Mtx view, Vec const& scale, const Mtx& base, Mtx dst) {
    Mtx m;

    m[0][0] = base[0][0] * scale.x;
    m[0][1] = base[0][1] * scale.y;
    m[0][2] = base[0][2] * scale.z;
    m[0][3] = base[0][3];

    m[1][0] = base[1][0] * scale.x;
    m[1][1] = base[1][1] * scale.y;
    m[1][2] = base[1][2] * scale.z;
    m[1][3] = base[1][3];

    m[2][0] = base[2][0] * scale.x;
    m[2][1] = base[2][1] * scale.y;
    m[2][2] = base[2][2] * scale.z;
    m[2][3] = base[2][3];

    PSMTXConcat(view, m, dst);
}
