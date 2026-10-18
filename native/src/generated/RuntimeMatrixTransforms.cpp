#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include <JSystem/JMath/JMATrigonometric.hpp>
#include <JSystem/JMath/JMath.hpp>
static Mtx tmpmtx_ry = {{1.0f,0.0f,0.0f,0.0f},{0.0f,1.0f,0.0f,0.0f},{0.0f,0.0f,1.0f,0.0f}};
namespace MR {
    void rotateMtxLocalX(MtxPtr pMtx, f32 rad) {
        f32 cosAngle = JMACosRadian(rad);
        f32 sinAngle = JMASinRadian(rad);
        f32 a0 = cosAngle * pMtx[0][1] - sinAngle * pMtx[0][2];
        f32 a1 = cosAngle * pMtx[1][1] - sinAngle * pMtx[1][2];
        f32 a2 = cosAngle * pMtx[2][1] - sinAngle * pMtx[2][2];
        f32 b0 = sinAngle * pMtx[0][1] + cosAngle * pMtx[0][2];
        f32 b1 = sinAngle * pMtx[1][1] + cosAngle * pMtx[1][2];
        f32 b2 = sinAngle * pMtx[2][1] + cosAngle * pMtx[2][2];
        pMtx[0][1] = a0;
        pMtx[1][1] = a1;
        pMtx[2][1] = a2;
        pMtx[0][2] = b0;
        pMtx[1][2] = b1;
        pMtx[2][2] = b2;
    }
    void rotateMtxLocalY(MtxPtr pMtx, f32 rad) {
        f32 cosAngle = JMACosRadian(rad);
        f32 sinAngle = JMASinRadian(rad);
        f32 a0 = cosAngle * pMtx[0][0] - sinAngle * pMtx[0][2];
        f32 a1 = cosAngle * pMtx[1][0] - sinAngle * pMtx[1][2];
        f32 a2 = cosAngle * pMtx[2][0] - sinAngle * pMtx[2][2];
        f32 b0 = sinAngle * pMtx[0][0] + cosAngle * pMtx[0][2];
        f32 b1 = sinAngle * pMtx[1][0] + cosAngle * pMtx[1][2];
        f32 b2 = sinAngle * pMtx[2][0] + cosAngle * pMtx[2][2];
        pMtx[0][0] = a0;
        pMtx[1][0] = a1;
        pMtx[2][0] = a2;
        pMtx[0][2] = b0;
        pMtx[1][2] = b1;
        pMtx[2][2] = b2;
    }
    void rotateMtxLocalZ(MtxPtr pMtx, f32 rad) {
        f32 cosAngle = JMACosRadian(rad);
        f32 sinAngle = JMASinRadian(rad);
        f32 a0 = cosAngle * pMtx[0][0] - sinAngle * pMtx[0][1];
        f32 a1 = cosAngle * pMtx[1][0] - sinAngle * pMtx[1][1];
        f32 a2 = cosAngle * pMtx[2][0] - sinAngle * pMtx[2][1];
        f32 b0 = sinAngle * pMtx[0][0] + cosAngle * pMtx[0][1];
        f32 b1 = sinAngle * pMtx[1][0] + cosAngle * pMtx[1][1];
        f32 b2 = sinAngle * pMtx[2][0] + cosAngle * pMtx[2][1];
        pMtx[0][0] = a0;
        pMtx[1][0] = a1;
        pMtx[2][0] = a2;
        pMtx[0][1] = b0;
        pMtx[1][1] = b1;
        pMtx[2][1] = b2;
    }
    void rotateMtxLocalXDegree(MtxPtr pMtx, f32 deg) {
        rotateMtxLocalX(pMtx, deg * PI_180);
    }
    void rotateMtxLocalYDegree(MtxPtr pMtx, f32 deg) {
        rotateMtxLocalY(pMtx, deg * PI_180);
    }
    void rotateMtxLocalZDegree(MtxPtr pMtx, f32 deg) {
        rotateMtxLocalZ(pMtx, deg * PI_180);
    }
    void orthogonalize(TPos3f* pMtx) {
        TVec3f axisZ, axisY, axisX;

        pMtx->getXYZDir(axisX, axisY, axisZ);

        TVec3f* pZ;
        TVec3f* pY;
        pY = &axisY;
        pZ = &axisZ;

        PSVECCrossProduct(pY, pZ, &axisX);
        PSVECCrossProduct(pZ, &axisX, pY);

        axisX.normalize();
        pY->normalize();

        pMtx->setXYZDir(axisX, *pY, *pZ);

        TVec3f zDir;
        pMtx->getZDir(zDir);

        JGeometry::TUtil< f32 >::sqrt((pMtx->get(0, 0) * pMtx->get(0, 0)) + (pMtx->get(1, 0) * pMtx->get(1, 0)) + (pMtx->get(2, 0) * pMtx->get(2, 0)) +
                                      (pMtx->get(0, 1) * pMtx->get(0, 1)) + (pMtx->get(1, 1) * pMtx->get(1, 1)) + (pMtx->get(2, 1) * pMtx->get(2, 1)) +
                                      (pMtx->get(0, 2) * pMtx->get(0, 2)) + (pMtx->get(1, 2) * pMtx->get(1, 2)) + (pMtx->get(2, 2) * pMtx->get(2, 2)));

        if (pMtx != nullptr) {
            f32 invLenX = JGeometry::TUtil< f32 >::inv_sqrt((pMtx->get(0, 0) * pMtx->get(0, 0)) + (pMtx->get(1, 0) * pMtx->get(1, 0)) +
                                                            (pMtx->get(2, 0) * pMtx->get(2, 0)));
            pMtx->mMtx[0][0] = invLenX * pMtx->get(0, 0);
            pMtx->mMtx[1][0] = invLenX * pMtx->get(1, 0);
            pMtx->mMtx[2][0] = invLenX * pMtx->get(2, 0);

            f32 invLenY = JGeometry::TUtil< f32 >::inv_sqrt((pMtx->get(0, 1) * pMtx->get(0, 1)) + (pMtx->get(1, 1) * pMtx->get(1, 1)) +
                                                            (pMtx->get(2, 1) * pMtx->get(2, 1)));
            pMtx->mMtx[0][1] = invLenY * pMtx->get(0, 1);
            pMtx->mMtx[1][1] = invLenY * pMtx->get(1, 1);
            pMtx->mMtx[2][1] = invLenY * pMtx->get(2, 1);

            f32 invLenZ = JGeometry::TUtil< f32 >::inv_sqrt((pMtx->get(0, 2) * pMtx->get(0, 2)) + (pMtx->get(1, 2) * pMtx->get(1, 2)) +
                                                            (pMtx->get(2, 2) * pMtx->get(2, 2)));
            pMtx->mMtx[0][2] = invLenZ * pMtx->get(0, 2);
            pMtx->mMtx[1][2] = invLenZ * pMtx->get(1, 2);
            pMtx->mMtx[2][2] = invLenZ * pMtx->get(2, 2);
        }

        pMtx->mMtx[0][2] = zDir.x;
        pMtx->mMtx[1][2] = zDir.y;
        pMtx->mMtx[2][2] = zDir.z;
    }
    void rotateMtxMoment(TPos3f* pDst, const TPos3f& rSrc, const TVec3f& rMoment) {
        if (rMoment.isZero()) {
            *pDst = rSrc;
        } else {
            TVec3f dir(rMoment);
            f32 mag;
            MR::separateScalarAndDirection(&mag, &dir, dir);

            TVec3f trans;
            rSrc.getTrans(trans);

            TPos3f rotMtx;
            rotMtx.makeRotate(dir, mag);
            rotMtx.concat(rotMtx, rSrc);
            orthogonalize(&rotMtx);
            rotMtx.setTrans(trans);
            *pDst = rotMtx;
        }
    }
    void rotateMtxMoment(TPos3f* pMtx, const TVec3f& rMoment) {
        rotateMtxMoment(pMtx, *pMtx, rMoment);
    }
    void addTransMtxLocal(MtxPtr pMtx, const TVec3f& rVec) {
        addTransMtxLocalX(pMtx, rVec.x);
        addTransMtxLocalY(pMtx, rVec.y);
        addTransMtxLocalZ(pMtx, rVec.z);
    }
    void addTransMtxLocalX(MtxPtr pMtx, f32 x_coord) {
        pMtx[0][3] = pMtx[0][3] + (pMtx[0][0] * x_coord);
        pMtx[1][3] = pMtx[1][3] + (pMtx[1][0] * x_coord);
        pMtx[2][3] = pMtx[2][3] + (pMtx[2][0] * x_coord);
    }
    void makeMtxFrontNoSupport(TPos3f* pDst, const TVec3f& rFront) {
        TVec3f support;
        if (MR::getMaxAbsElementIndex(rFront) == 1) {
            support.set< f32 >(1.0f, 0.0f, 0.0f);
        } else {
            support.set< f32 >(0.0f, 1.0f, 0.0f);
        }

        TVec3f axisX, axisY, axisZ;
        MR::normalize(rFront, &axisZ);

        axisX.cross(support, axisZ);
        MR::normalize(&axisX);

        axisY.cross(axisZ, axisX);

        pDst->setXYZDir(axisX, axisY, axisZ);
    }
    void makeMtxUpSide(TPos3f* pDst, const TVec3f& rUp, const TVec3f& rSide) {
        TVec3f axisZ, axisY, axisX;
        MR::normalize(rUp, &axisY);

        axisZ.cross(rSide, axisY);
        MR::normalize(&axisZ);

        axisX.cross(axisY, axisZ);

        pDst->setXYZDir(axisX, axisY, axisZ);
    }
    void preScaleMtx(MtxPtr pMtx, f32 scale) {
        preScaleMtx(pMtx, scale, scale, scale);
    }
    void preScaleMtx(MtxPtr pMtx, const TVec3f& rScale) {
        preScaleMtx(pMtx, rScale.x, rScale.y, rScale.z);
    }
    void preScaleMtx(MtxPtr pMtx, f32 sx, f32 sy, f32 sz) {
        pMtx[0][0] *= sx;
        pMtx[1][0] *= sx;
        pMtx[2][0] *= sx;
        pMtx[0][1] *= sy;
        pMtx[1][1] *= sy;
        pMtx[2][1] *= sy;
        pMtx[0][2] *= sz;
        pMtx[1][2] *= sz;
        pMtx[2][2] *= sz;
    }
    void flattenMtx(MtxPtr pDst, MtxPtr pSrc, const TVec3f& rNormal) {
        TVec3f axisX, axisY, axisZ;
        MR::extractMtxXDir(pSrc, &axisX);
        MR::extractMtxYDir(pSrc, &axisY);
        MR::extractMtxZDir(pSrc, &axisZ);

        axisX.orthogonalize(rNormal);
        axisY.orthogonalize(rNormal);
        axisZ.orthogonalize(rNormal);

        MR::setMtxAxisXYZ(pDst, axisX, axisY, axisZ);
    }
    void flattenMtx(MtxPtr pMtx, const TVec3f& rNormal) {
        flattenMtx(pMtx, pMtx, rNormal);
    }
    void makeMtxUpNoSupport(TPos3f* pDst, const TVec3f& rUp) {
        TVec3f support;
        if (MR::getMaxAbsElementIndex(rUp) == 2) {
            support.set< f32 >(0.0f, 1.0f, 0.0f);
        } else {
            support.set< f32 >(0.0f, 0.0f, 1.0f);
        }

        TVec3f axisX, axisY, axisZ;
        MR::normalize(rUp, &axisY);

        axisX.cross(axisY, support);
        MR::normalize(&axisX);

        axisZ.cross(axisX, axisY);

        pDst->setXYZDir(axisX, axisY, axisZ);
    }
    void addTransMtx(MtxPtr pMtx, const TVec3f& rVec) {
        pMtx[0][3] += rVec.x;
        pMtx[1][3] += rVec.y;
        pMtx[2][3] += rVec.z;
    }
    void addTransMtxLocalY(MtxPtr pMtx, f32 y_coord) {
        pMtx[0][3] = pMtx[0][3] + (pMtx[0][1] * y_coord);
        pMtx[1][3] = pMtx[1][3] + (pMtx[1][1] * y_coord);
        pMtx[2][3] = pMtx[2][3] + (pMtx[2][1] * y_coord);
    }
    void addTransMtxLocalZ(MtxPtr pMtx, f32 z_coord) {
        pMtx[0][3] = pMtx[0][3] + (pMtx[0][2] * z_coord);
        pMtx[1][3] = pMtx[1][3] + (pMtx[1][2] * z_coord);
        pMtx[2][3] = pMtx[2][3] + (pMtx[2][2] * z_coord);
    }
    void blendMtx(MtxPtr pMtxA, MtxPtr pMtxB, f32 blend, MtxPtr pDst) {
        TVec3f transA, transB, transR;
        extractMtxTrans(pMtxA, &transA);
        extractMtxTrans(pMtxB, &transB);

        transR = transA * (1.0f - blend) + transB * blend;

        blendMtxRotate(pMtxA, pMtxB, blend, pDst);
        setMtxTrans(pDst, transR.x, transR.y, transR.z);
    }
    void blendMtxRotate(MtxPtr pMtxA, MtxPtr pMtxB, f32 blend, MtxPtr pDst) {
        Quaternion quatA, quatB, quatR;
        C_QUATMtx(&quatA, pMtxA);
        C_QUATMtx(&quatB, pMtxB);
        JMAQuatLerp(&quatA, &quatB, blend, &quatR);
        PSMTXQuat(pDst, &quatR);
    }
    MtxPtr tmpMtxRotYRad(f32 rad) {
        f32 cosY = JMACosRadian(rad);
        f32 sinY = JMASinRadian(rad);
        tmpmtx_ry[0][0] = cosY;
        tmpmtx_ry[0][2] = sinY;
        tmpmtx_ry[2][0] = -sinY;
        tmpmtx_ry[2][2] = cosY;
        return tmpmtx_ry;
    }
    void makeRTFromMtxPtr(TVec3f* pOutTrans, TVec3f* pOutRot, MtxPtr pSrc, bool toDegree) {
        TPos3f* pMtx = reinterpret_cast< TPos3f* >(pSrc);
        if (pOutTrans != nullptr) {
            pMtx->getTrans(*pOutTrans);
        }

        if (pOutRot != nullptr) {
            pMtx->getEuler(*pOutRot);

            if (toDegree) {
                pOutRot->set(*pOutRot * _180_PI);
            }
        }
    }
    void makeMtxFrontUp(TPos3f* pDst, const TVec3f& rFront, const TVec3f& rUp) {
        TVec3f axisZ, axisY, axisX;
        MR::normalize(rFront, &axisZ);

        axisX.cross(rUp, axisZ);
        MR::normalize(&axisX);

        axisY.cross(axisZ, axisX);

        pDst->setXYZDir(axisX, axisY, axisZ);
    }
    void makeMtxFrontUpPos(TPos3f* pDst, const TVec3f& rFront, const TVec3f& rUp, const TVec3f& rPos) {
        makeMtxFrontUp(pDst, rFront, rUp);
        pDst->setTrans(rPos);
    }
}

