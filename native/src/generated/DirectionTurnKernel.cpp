#include <Game/Util/MathUtil.hpp>
#include <Game/Util/MtxUtil.hpp>
#include <JSystem/JGeometry/TUtil.hpp>
namespace MR {
    bool makeAxisAndCosignVecToVec(TVec3f* pAxis, f32* pCos, const TVec3f& rFrom, const TVec3f& rTo) {
        TVec3f from;
        if (isOppositeDirection(rFrom, rTo)) {
            turnRandomVector(&from, rFrom, 0.01f);
            normalize(&from);
        } else {
            from.set(rFrom);
        }

        TVec3f axis = from.cross(rTo);

        if (isNearZero(axis)) {
            pAxis->zero();
            *pCos = 1.0f;

            return false;
        } else {
            normalize(axis, pAxis);
            *pCos = JGeometry::TUtil< f32 >::clamp(from.dot(rTo), -1.0f, 1.0f);

            return true;
        }
    }
    f32 turnVecToPlane(TVec3f* pDst, const TVec3f& rSrc, const TVec3f& rNorm) {
        TVec3f vec = rSrc;
        f32 length = rSrc.length();
        f32 proj = rNorm.dot(rSrc);
        vec.sub(rNorm * proj);
        vec.setLength(length);
        pDst->set(vec);
        return length;
    }
    bool turnVecToVecCosOnPlane(TVec3f* pDst, const TVec3f& rFrom, const TVec3f& rTo, const TVec3f& rAxis, f32 cosAngle) {
        TVec3f planarTo = rTo.killElement(rAxis);
        normalizeOrZero(&planarTo);
        if (!MR::isNearZero(planarTo)) {
            if (cosAngle > -1.0f) {
                return turnVecToVecCos(pDst, rFrom, planarTo, cosAngle, rAxis);
            }

            pDst->set(planarTo);
            return true;
        }

        return false;
    }
    bool turnVecToVecCosOnPlane(TVec3f* pVec, const TVec3f& rTo, const TVec3f& rAxis, f32 cosAngle) {
        return turnVecToVecCosOnPlane(pVec, *pVec, rTo, rAxis, cosAngle);
    }
    void rotateQuatRollBall(TQuat4f* pQuat, const TVec3f& rVel, const TVec3f& rNorm, f32 radius) {
        TVec3f moment;
        calcMomentRollBall(&moment, rVel, rNorm, radius);
        rotateQuatMoment(pQuat, moment);
    }
    void addRandomVector(TVec3f* pDst, const TVec3f& rSrc, f32 range) {
        f32 x = getRandom(-range, range);
        f32 y = getRandom(-range, range);
        f32 z = getRandom(-range, range);

        pDst->set(rSrc + TVec3f(x, y, z));
    }
    void turnRandomVector(TVec3f* pDst, const TVec3f& rSrc, f32 range) {
        f32 srcLength = rSrc.length();

        addRandomVector(pDst, rSrc, range);

        if (isNearZero(*pDst)) {
            pDst->set(rSrc);
        } else {
            pDst->setLength(srcLength);
        }
    }
    bool turnQuat(TQuat4f* pDst, const TQuat4f& rSrc, const TVec3f& rFrom, const TVec3f& rTo, f32 angle) {
        TVec3f from;
        TVec3f target = rTo;

        bool isSameDir = rFrom.dot(rTo) >= 0.0f ? false : isSameDirection(rFrom, rTo);
        if (isSameDir) {
            turnRandomVector(&from, rFrom, 0.001f);
        } else {
            from.set(rFrom);
        }

        MR::normalizeOrZero(&from);
        MR::normalizeOrZero(&target);

        f32 angleBetween = acosEx(JGeometry::TUtil< f32 >::clamp(from.dot(target), -1.0f, 1.0f));
        f32 rate;
        if (angleBetween <= angle) {
            rate = 1.0f;
        } else {
            rate = JGeometry::TUtil< f32 >::clamp(angle / angleBetween, 0.0f, 1.0f);
        }

        TQuat4f rot;
        rot.setRotate(from, target, rate);
        pDst->mult(rot, rSrc);
        pDst->normalize();

        return angleBetween < 0.015f;
    }
    bool turnQuatXDirRad(TQuat4f* pDst, const TQuat4f& rSrc, const TVec3f& rTo, f32 angle) {
        TVec3f xDir;
        rSrc.getXDir(xDir);
        return turnQuat(pDst, rSrc, xDir, rTo, angle);
    }
    void turnMtxToXDirDegree(TPos3f* pMtx, const TVec3f& rDir, f32 degree) {
        TQuat4f quat;
        pMtx->getQuat(quat);
        MR::turnQuatXDirRad(&quat, quat, rDir, PI_180 * degree);
        pMtx->setQuat(quat);
    }
    void turnMtxToLocalDirDegree(TPos3f* pMtx, const TVec3f& rLocalAxis, const TVec3f& rDir, f32 degree) {
        TVec3f worldAxis;
        pMtx->mult33(rLocalAxis, worldAxis);

        TQuat4f quat;
        pMtx->getQuat(quat);
        MR::turnQuat(&quat, quat, worldAxis, rDir, PI_180 * degree);
        pMtx->setQuat(quat);
    }
}
