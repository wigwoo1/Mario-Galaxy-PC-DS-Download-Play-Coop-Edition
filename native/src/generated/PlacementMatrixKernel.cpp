#include <Game/LiveActor/LiveActor.hpp>
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include <JSystem/JMath/JMATrigonometric.hpp>
#include <JSystem/JMath/JMath.hpp>

static Mtx mtrans_org = {{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}};

static Mtx tmpmtx_sc = {{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}};

static Mtx tmpmtx_rx = {{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}};

static Mtx tmpmtx_ry = {{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}};

static Mtx tmpmtx_rz = {{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}};


namespace MR {
    void makeMtxRotate(MtxPtr pMtx, s16 rx, s16 ry, s16 rz) {
        f32 sinX = JMASSin(rx);
        f32 sinY = JMASSin(ry);
        f32 sinZ = JMASSin(rz);
        f32 cosX = JMASCos(rx);
        f32 cosY = JMASCos(ry);
        f32 cosZ = JMASCos(rz);

        pMtx[0][0] = cosZ * cosY;
        pMtx[1][0] = sinZ * cosY;
        pMtx[2][0] = -sinY;

        pMtx[0][1] = cosZ * sinY * sinX - sinZ * cosX;
        pMtx[1][1] = sinZ * sinY * sinX + cosZ * cosX;
        pMtx[2][1] = cosY * sinX;

        pMtx[0][2] = cosZ * sinY * cosX + sinZ * sinX;
        pMtx[1][2] = sinZ * sinY * cosX - cosZ * sinX;
        pMtx[2][2] = cosY * cosX;

        pMtx[0][3] = 0.0f;
        pMtx[1][3] = 0.0f;
        pMtx[2][3] = 0.0f;
    }
    void makeMtxRotate(MtxPtr pMtx, f32 rx, f32 ry, f32 rz) {
        s16 angleX = (s16)(rx * DEGREE_TO_S16);
        s16 angleY = (s16)(ry * DEGREE_TO_S16);
        s16 angleZ = (s16)(rz * DEGREE_TO_S16);
        makeMtxRotate(pMtx, angleX, angleY, angleZ);
    }
    void makeMtxRotate(MtxPtr pMtx, const TVec3f& rVec) {
        makeMtxRotate(pMtx, rVec.x, rVec.y, rVec.z);
    }
    void makeMtxRotateY(MtxPtr pMtx, f32 ry) {
        s16 angle = (s16)(ry * DEGREE_TO_S16);
        f32 sinY = JMASSin(angle);
        f32 cosY = JMASCos(angle);

        pMtx[0][0] = cosY;
        pMtx[1][0] = 0.0f;
        pMtx[2][0] = -sinY;
        pMtx[0][1] = 0.0f;
        pMtx[1][1] = 1.0f;
        pMtx[2][1] = 0.0f;
        pMtx[0][2] = sinY;
        pMtx[1][2] = 0.0f;
        pMtx[2][2] = cosY;
        pMtx[0][3] = 0.0f;
        pMtx[1][3] = 0.0f;
        pMtx[2][3] = 0.0f;
    }
    void makeMtxTR(MtxPtr pMtx, f32 tx, f32 ty, f32 tz, f32 rx, f32 ry, f32 rz) {
        f32 sinX = JMASinDegree(rx);
        f32 sinY = JMASinDegree(ry);
        f32 sinZ = JMASinDegree(rz);
        f32 cosX = JMACosDegree(rx);
        f32 cosY = JMACosDegree(ry);
        f32 cosZ = JMACosDegree(rz);

        pMtx[0][0] = cosZ * cosY;
        pMtx[1][0] = sinZ * cosY;
        pMtx[2][0] = -sinY;

        pMtx[0][1] = cosZ * sinY * sinX - sinZ * cosX;
        pMtx[1][1] = sinZ * sinY * sinX + cosZ * cosX;
        pMtx[2][1] = cosY * sinX;

        pMtx[0][2] = cosZ * sinY * cosX + sinZ * sinX;
        pMtx[1][2] = sinZ * sinY * cosX - cosZ * sinX;
        pMtx[2][2] = cosY * cosX;

        pMtx[0][3] = tx;
        pMtx[1][3] = ty;
        pMtx[2][3] = tz;
    }
    void makeMtxTR(MtxPtr pMtx, const TVec3f& rTrans, const TVec3f& rRot) {
        makeMtxTR(pMtx, rTrans.x, rTrans.y, rTrans.z, rRot.x, rRot.y, rRot.z);
    }
    MtxPtr tmpMtxRotXDeg(f32 deg) {
        f32 cosX = JMACosDegree(deg);
        f32 sinX = JMASinDegree(deg);
        tmpmtx_rx[1][1] = cosX;
        tmpmtx_rx[2][1] = sinX;
        tmpmtx_rx[1][2] = -sinX;
        tmpmtx_rx[2][2] = cosX;
        return tmpmtx_rx;
    }
    MtxPtr tmpMtxRotYDeg(f32 deg) {
        f32 cosY = JMACosDegree(deg);
        f32 sinY = JMASinDegree(deg);
        tmpmtx_ry[0][0] = cosY;
        tmpmtx_ry[0][2] = sinY;
        tmpmtx_ry[2][0] = -sinY;
        tmpmtx_ry[2][2] = cosY;
        return tmpmtx_ry;
    }
    MtxPtr tmpMtxRotZDeg(f32 deg) {
        f32 cosZ = JMACosDegree(deg);
        f32 sinZ = JMASinDegree(deg);
        tmpmtx_rz[0][0] = cosZ;
        tmpmtx_rz[1][0] = sinZ;
        tmpmtx_rz[0][1] = -sinZ;
        tmpmtx_rz[1][1] = cosZ;
        return tmpmtx_rz;
    }
    void makeMtxTRS(MtxPtr pMtx, f32 tx, f32 ty, f32 tz, f32 rx, f32 ry, f32 rz, f32 sx, f32 sy, f32 sz) {
        f32 sinX = JMASinDegree(rx);
        f32 sinY = JMASinDegree(ry);
        f32 sinZ = JMASinDegree(rz);
        f32 cosX = JMACosDegree(rx);
        f32 cosY = JMACosDegree(ry);
        f32 cosZ = JMACosDegree(rz);

        pMtx[0][0] = sx * (cosZ * cosY);
        pMtx[1][0] = sx * (sinZ * cosY);
        pMtx[2][0] = -sinY * sx;

        pMtx[0][1] = sy * (cosZ * sinY * sinX - sinZ * cosX);
        pMtx[1][1] = sy * (sinZ * sinY * sinX + cosZ * cosX);
        pMtx[2][1] = sy * (cosY * sinX);

        pMtx[0][2] = sz * (cosZ * sinY * cosX + sinZ * sinX);
        pMtx[1][2] = sz * (sinZ * sinY * cosX - cosZ * sinX);
        pMtx[2][2] = sz * (cosY * cosX);

        pMtx[0][3] = tx;
        pMtx[1][3] = ty;
        pMtx[2][3] = tz;
    }
    void makeMtxTRS(MtxPtr pMtx, const TVec3f& rTrans, const TVec3f& rRot, const TVec3f& rScale) {
        makeMtxTRS(pMtx, rTrans.x, rTrans.y, rTrans.z, rRot.x, rRot.y, rRot.z, rScale.x, rScale.y, rScale.z);
    }
    void makeMtxTransRotateY(MtxPtr pMtx, f32 tx, f32 ty, f32 tz, f32 ry) {
        makeMtxRotateY(pMtx, ry);
        pMtx[0][3] = tx;
        pMtx[1][3] = ty;
        pMtx[2][3] = tz;
    }
    void makeMtxTR(MtxPtr pMtx, const LiveActor* pActor) {
        MR::makeMtxTR(pMtx, pActor->mPosition.x, pActor->mPosition.y, pActor->mPosition.z, pActor->mRotation.x, pActor->mRotation.y,
                      pActor->mRotation.z);
    }
    void makeMtxTRS(MtxPtr pMtx, const LiveActor* pActor) {
        MR::makeMtxTRS(pMtx, pActor->mPosition.x, pActor->mPosition.y, pActor->mPosition.z, pActor->mRotation.x, pActor->mRotation.y,
                       pActor->mRotation.z, pActor->mScale.x, pActor->mScale.y, pActor->mScale.z);
    }
    void makeMtxTransRotateY(MtxPtr pMtx, const LiveActor* pActor) {
        MR::makeMtxTransRotateY(pMtx, pActor->mPosition.x, pActor->mPosition.y, pActor->mPosition.z, pActor->mRotation.y);
    }
    void makeMtxWithoutScale(TPos3f* pDst, const TPos3f& rSrc) {
        TVec3f trans;
        rSrc.getTrans(trans);

        TVec3f axisX, axisY, axisZ;
        rSrc.getXYZDir(axisX, axisY, axisZ);

        pDst->setTrans(trans);

        MR::normalize(&axisX);
        MR::normalize(&axisY);
        MR::normalize(&axisZ);

        pDst->setXYZDir(axisX, axisY, axisZ);
    }
    void makeMtxSideUp(TPos3f* pDst, const TVec3f& rSide, const TVec3f& rUp) {
        TVec3f axisZ, axisY, axisX;
        MR::normalize(rSide, &axisX);

        axisZ.cross(axisX, rUp);
        MR::normalize(&axisZ);

        axisY.cross(axisZ, axisX);

        pDst->setXYZDir(axisX, axisY, axisZ);
    }
    void makeMtxSideUpPos(TPos3f* pDst, const TVec3f& rSide, const TVec3f& rUp, const TVec3f& rPos) {
        makeMtxSideUp(pDst, rSide, rUp);
        pDst->setTrans(rPos);
    }
    void extractMtxXDir(MtxPtr pMtx, TVec3f* pOut) {
        pOut->x = pMtx[0][0];
        pOut->y = pMtx[1][0];
        pOut->z = pMtx[2][0];
    }
    void extractMtxYDir(MtxPtr pMtx, TVec3f* pOut) {
        pOut->x = pMtx[0][1];
        pOut->y = pMtx[1][1];
        pOut->z = pMtx[2][1];
    }
    void extractMtxZDir(MtxPtr pMtx, TVec3f* pOut) {
        pOut->x = pMtx[0][2];
        pOut->y = pMtx[1][2];
        pOut->z = pMtx[2][2];
    }
}
