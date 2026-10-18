#include <Game/Util/MathUtil.hpp>
#include <Game/Util/MtxUtil.hpp>
namespace MR {
#ifndef GALAXY_SHARED_RUNTIME_MATH
    void normalize(TVec3f* pVec) {
        Vec input{pVec->x,pVec->y,pVec->z},output;PSVECNormalize(&input,&output);pVec->set(output.x,output.y,output.z);
    }
#endif
#ifndef GALAXY_SHARED_RUNTIME_MATH
    void normalize(const TVec3f& rSrc, TVec3f* pDst) {
        pDst->set(rSrc);
        normalize(pDst);
    }
#endif
    u32 getMaxAbsElementIndex(const TVec3f& rVec) {
        f64 abs_x = abs(rVec.x);
        f64 abs_y = abs(rVec.y);
        f64 abs_z = abs(rVec.z);

        if (abs_x > abs_y && abs_x > abs_z) {
            return 0;
        }

        if (abs_y > abs_z) {
            return 1;
        }

        return 2;
    }
    void makeAxisUpSide(TVec3f* pFront, TVec3f* pSide, const TVec3f& rUp, const TVec3f& rSide) {
        pFront->cross(rSide, rUp);
        normalize(pFront);
        pSide->cross(rUp, *pFront);
    }
    void makeQuatFromVec(TQuat4f* pQuat, const TVec3f& rFront, const TVec3f& rUp) {
        TPos3f mtx;
        makeMtxFrontUp(&mtx, rFront, rUp);
        mtx.getQuat(*pQuat);
    }
    void makeQuatUpFront(TQuat4f* pQuat, const TVec3f& rUp, const TVec3f& rFront) {
        TPos3f mtx;
        makeMtxUpFront(&mtx, rUp, rFront);
        mtx.getQuat(*pQuat);
    }
    void makeMtxFrontSidePos(TPos3f* pDst, const TVec3f& rFront, const TVec3f& rSide, const TVec3f& rPos) {
        TVec3f axisX, axisY, axisZ;
        MR::normalize(rFront, &axisZ);

        axisY.cross(axisZ, rSide);
        MR::normalize(&axisY);

        axisX.cross(axisY, axisZ);

        pDst->mMtx[0][0]=axisX.x;pDst->mMtx[1][0]=axisX.y;pDst->mMtx[2][0]=axisX.z;pDst->mMtx[0][1]=axisY.x;pDst->mMtx[1][1]=axisY.y;pDst->mMtx[2][1]=axisY.z;pDst->mMtx[0][2]=axisZ.x;pDst->mMtx[1][2]=axisZ.y;pDst->mMtx[2][2]=axisZ.z;
        pDst->setTrans(rPos);
    }
    void makeMtxFrontUp(TPos3f* pDst, const TVec3f& rFront, const TVec3f& rUp) {
        TVec3f axisZ, axisY, axisX;
        MR::normalize(rFront, &axisZ);

        axisX.cross(rUp, axisZ);
        MR::normalize(&axisX);

        axisY.cross(axisZ, axisX);

        pDst->mMtx[0][0]=axisX.x;pDst->mMtx[1][0]=axisX.y;pDst->mMtx[2][0]=axisX.z;pDst->mMtx[0][1]=axisY.x;pDst->mMtx[1][1]=axisY.y;pDst->mMtx[2][1]=axisY.z;pDst->mMtx[0][2]=axisZ.x;pDst->mMtx[1][2]=axisZ.y;pDst->mMtx[2][2]=axisZ.z;
    }
    void makeMtxUpFront(TPos3f* pDst, const TVec3f& rUp, const TVec3f& rFront) {
        TVec3f axisZ, axisY, axisX;
        MR::normalize(rUp, &axisY);

        axisX.cross(axisY, rFront);
        MR::normalize(&axisX);

        axisZ.cross(axisX, axisY);

        pDst->mMtx[0][0]=axisX.x;pDst->mMtx[1][0]=axisX.y;pDst->mMtx[2][0]=axisX.z;pDst->mMtx[0][1]=axisY.x;pDst->mMtx[1][1]=axisY.y;pDst->mMtx[2][1]=axisY.z;pDst->mMtx[0][2]=axisZ.x;pDst->mMtx[1][2]=axisZ.y;pDst->mMtx[2][2]=axisZ.z;
    }
    void makeMtxUpFrontPos(TPos3f* pDst, const TVec3f& rUp, const TVec3f& rFront, const TVec3f& rPos) {
        makeMtxUpFront(pDst, rUp, rFront);
        pDst->setTrans(rPos);
    }
    void makeMtxUpNoSupportPos(TPos3f* pDst, const TVec3f& rUp, const TVec3f& rPos) {
        TVec3f support;
        if (MR::getMaxAbsElementIndex(rUp) == 2) {
            support.set< f32 >(0.0f, 1.0f, 0.0f);
        } else {
            support.set< f32 >(0.0f, 0.0f, 1.0f);
        }

        MR::makeMtxUpFrontPos(pDst, rUp, support, rPos);
    }

}

