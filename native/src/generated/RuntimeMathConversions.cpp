#include "Game/Util/MathUtil.hpp"
#include <JSystem/JGeometry/TUtil.hpp>
#include <JSystem/JMath/JMATrigonometric.hpp>
#include "Game/Util/MtxUtil.hpp"
#include <cmath>
#include <revolution/mtx.h>
const Vec gZeroVec = {0.0f, 0.0f, 0.0f};
namespace MR {
    bool turnQuatYDirRad(TQuat4f* pDst, const TQuat4f& rSrc, const TVec3f& rTo, f32 angle) {
        TVec3f yDir;
        rSrc.getYDir(yDir);
        return turnQuat(pDst, rSrc, yDir, rTo, angle);
    }
    bool turnQuatZDirRad(TQuat4f* pDst, const TQuat4f& rSrc, const TVec3f& rTo, f32 angle) {
        TVec3f zDir;
        rSrc.getZDir(zDir);
        return turnQuat(pDst, rSrc, zDir, rTo, angle);
    }
    void clampVecAngleDeg(TVec3f* pDst, const TVec3f& rBase, f32 angle) {
        if (pDst->angle(rBase) * _180_PI <= angle) {
            return;
        }

        TVec3f axis = rBase.cross(*pDst);
        if (MR::normalizeOrZero(&axis)) {
            return;
        }

        f32 degree = JMACosDegree(angle);

        rotateVecDegree(pDst, rBase, axis, angle);
    }
    void normalize(TPos3f* pMtx) {
        TVec3f x, y, z;
        pMtx->getXYZDir(x, y, z);
        normalize(&x);
        normalize(&y);
        normalize(&z);
        pMtx->mMtx[0][0]=x.x; pMtx->mMtx[1][0]=x.y; pMtx->mMtx[2][0]=x.z; pMtx->mMtx[0][1]=y.x; pMtx->mMtx[1][1]=y.y; pMtx->mMtx[2][1]=y.z; pMtx->mMtx[0][2]=z.x; pMtx->mMtx[1][2]=z.y; pMtx->mMtx[2][2]=z.z;
    }
    void makeQuatRotateRadian(TQuat4f* pQuat, const TVec3f& rRot) {
        pQuat->setEuler(rRot.x, rRot.y, rRot.z);
    }
    bool isOppositeDirection(const TVec3f& rVec1, const TVec3f& rVec2, f32 tolerance) {
        if (rVec1.dot(rVec2) >= 0.0f) {
            return false;
        }

        return isSameDirection(rVec1, rVec2, tolerance);
    }
    f32 diffAngleSignedHorizontal(const TVec3f& rA, const TVec3f& rB, const TVec3f& rAxis) {
        TVec3f horizonA, horizonB;
        vecKillElement(rA, rAxis, &horizonA);
        vecKillElement(rB, rAxis, &horizonB);
        f32 angleAbs = diffAngleAbs(horizonA, horizonB);
        TPos3f mtx;
        { Vec axis{rAxis.x,rAxis.y,rAxis.z}; PSMTXRotAxisRad(mtx.toMtxPtr(), &axis, 0.00017453293f); }
        f32 angle1 = horizonA.dot(horizonB);
        { Vec input{horizonA.x,horizonA.y,horizonA.z}, output; PSMTXMultVec(mtx.toMtxPtr(), &input, &output); horizonA.set(output.x,output.y,output.z); }
        f32 angle2 = horizonA.dot(horizonB);

        if (angle2 > angle1) {
            return angleAbs;
        } else {
            return -angleAbs;
        }
    }
    void createBoundingBox(const TVec3f* pPoints, u32 numPoints, TVec3f* pMin, TVec3f* pMax) {
        *pMin = pPoints[0];
        *pMax = pPoints[0];

        for (u32 idx = 1; idx < numPoints; idx++) {
            const TVec3f& point = pPoints[idx];

            if (point.x < pMin->x) {
                pMin->x = point.x;
            }

            if (point.y < pMin->y) {
                pMin->y = point.y;
            }

            if (point.z < pMin->z) {
                pMin->z = point.z;
            }

            if (pMax->x < point.x) {
                pMax->x = point.x;
            }

            if (pMax->y < point.y) {
                pMax->y = point.y;
            }

            if (pMax->z < point.z) {
                pMax->z = point.z;
            }
        }
    }
    f32 getLinerValueFromMinMax(f32 x, f32 min, f32 max, f32 start, f32 end) {
        return getInterpolateValue((JGeometry::TUtil< f32 >::clamp(x, min, max) - min) / (max - min), start, end);
    }
    bool turnVecToVecCos(TVec3f* pDst, const TVec3f& rFrom, const TVec3f& rTo, f32 cosAngle, const TVec3f& rAxis, f32 fallbackLength) {
        if (isNearZero(rFrom)) {
            return false;
        }

        if (isNearZero(rTo)) {
            return false;
        }

        if (rFrom.dot(rTo) > cosAngle) {
            pDst->set(rTo);
            normalize(pDst);
            return true;
        }

        f32 sinAngle = JMASqrt(1.0f - cosAngle * cosAngle);
        TVec3f rejectFrom = rTo.killElement(rFrom);

        if (isNearZero(rejectFrom)) {
            TVec3f fallback = rFrom.cross(rAxis);
            normalize(&fallback);
            pDst->set(rFrom + fallback * fallbackLength);
            normalize(pDst);
            return false;
        }

        normalize(&rejectFrom);
        pDst->set(rFrom * cosAngle + rejectFrom * sinAngle);
        normalize(pDst);
        return false;
    }
    u32 getMinAbsElementIndex(const TVec3f& rVec) {
        f64 abs_x = abs(rVec.x);
        f64 abs_y = abs(rVec.y);
        f64 abs_z = abs(rVec.z);

        if (abs_x < abs_y && abs_x < abs_z) {
            return 0;
        }

        if (abs_y < abs_z) {
            return 1;
        }

        return 2;
    }
    f32 sign(f32 x) {
        if (0.0f < x) {
            return 1.0f;
        }

        if (x < 0.0f) {
            return -1.0f;
        }

        return 0.0f;
    }
    bool isNan(const TVec3f& rVec) {
        if (std::isnan(rVec.x) || std::isnan(rVec.y) || std::isnan(rVec.z)) {
            return true;
        }

        return false;
    }
    f32 getFootPoint(const TVec3f& rPointA, const TVec3f& rPointB, const TVec3f& rPos, TVec3f* pDst) {
        TVec3f segment = rPointB - rPointA;
        f32 length = segment.length();
        f32 proj = -(segment.x * (rPointA.x - rPos.x) + segment.y * (rPointA.y - rPos.y) + segment.z * (rPointA.z - rPos.z)) / (length * length);
        *pDst = rPointA + segment * proj;
        return proj;
    }
    bool normalizeOrZero(const TVec3f& rSrc, TVec3f* pDst) {
        pDst->set(rSrc);

        return normalizeOrZero(pDst);
    }
    u8 lerp(u8 start, u8 end, f32 t) {
        return JGeometry::TUtil< f32 >::clamp(start + (end - start) * t, 0.0f, 255.0f);
    }
    void fixed16ToFloat(TVec3f* pDst, const TVec3s& rPSrc, u8 q) {
        f32 scale = 1.0f / (1 << q);

        pDst->x = rPSrc.x * scale;
        pDst->y = rPSrc.y * scale;
        pDst->z = rPSrc.z * scale;
    }
    void floatToFixed16(TVec3s* pDst, const TVec3f& rPSrc, u8 q) {
        f32 scale = 1 << q;

        pDst->x = rPSrc.x * scale;
        pDst->y = rPSrc.y * scale;
        pDst->z = rPSrc.z * scale;
    }
    GXColor lerp(GXColor start, GXColor end, f32 t) {
        u8 a = lerp(start.a, end.a, t);
        u8 b = lerp(start.b, end.b, t);
        u8 g = lerp(start.g, end.b, t);
        u8 r = lerp(start.r, end.r, t);

        GXColor color = {r, g, b, a};

        return color;
    }
    void getRotatedAxisY(TVec3f* pDst, const TVec3f& rPSrc) {
        f32 var = rPSrc.x;
        f32 CosX = JMACosDegree(var);
        var = rPSrc.y;
        f32 CosY = JMACosDegree(var);
        var = rPSrc.z;
        f32 CosZ = JMACosDegree(var);
        var = rPSrc.x;
        f32 SinX = JMASinDegree(var);
        var = rPSrc.y;
        f32 SinY = JMASinDegree(var);
        var = rPSrc.z;
        f32 SinZ = JMASinDegree(var);

        pDst->set((SinX * (CosZ * SinY)) - (SinZ * CosX), (SinX * (SinZ * SinY)) + (CosZ * CosX), CosY * SinX);
    }
    void getRotatedAxisZ(TVec3f* pDst, const TVec3f& rPSrc) {
        TVec3f vec(0.0f, 0.0f, 1.0f);
        TPos3f mtx;
        MR::makeMtxTR(mtx, 0.0f, 0.0f, 0.0f, rPSrc.x, rPSrc.y, rPSrc.z);
        PSMTXMultVec(mtx, &vec, pDst);
    }
    bool calcReboundVelocity(TVec3f* pVel, const TVec3f& rNormal, f32 reboundRateV, f32 reboundRateH) {
        f32 dot = pVel->dot(rNormal);
        if (dot < 0.0f) {
            pVel->sub(rNormal * dot);
            pVel->scale(reboundRateH);
            pVel->sub(rNormal * dot * reboundRateV);
            return true;
        }

        return false;
    }
    bool calcReboundVelocity(TVec3f* pVel, const TVec3f& rNormal, f32 reboundRate) {
        f32 dot = pVel->dot(rNormal);
        if (dot < 0.0f) {
            f32 rate = reboundRate + 1.0f;
            pVel->x -= rate * rNormal.x * dot;
            pVel->y -= rate * rNormal.y * dot;
            pVel->z -= rate * rNormal.z * dot;
            return true;
        }

        return false;
    }
    bool calcReflectionVector(TVec3f* pDir, const TVec3f& rNorm, f32 rate, f32 cosAngleMin) {
        f32 dot = rNorm.dot(*pDir);
        if (dot < -cosAngleMin) {
            pDir->sub(rNorm * dot * (rate + 1.0f));
            return true;
        }

        if (dot < 0.0f) {
            pDir->sub(rNorm * dot);
            return false;
        }

        return false;
    }
    void calcParabolicFunctionParam(f32* pAccel, f32* pVel, f32 max, f32 end) {
        f32 discrim = max * (max - end);

        if (discrim <= 0.0f) {
            *pAccel = -max;
            *pVel = end + max;
            return;
        }

        if (MR::abs(end) < 0.0001f) {
            *pAccel = max * -4.0f;
            *pVel = max * 4.0f;
            return;
        }

        f32 discRoot = JMASqrt(discrim);

        f32 t1 = (max + discRoot) / end;
        f32 t2 = (max - discRoot) / end;

        f32 t;
        if (0.0f <= t1 && t1 <= 1.0f) {
            t = t1;
        } else if (0.0f <= t2 && t2 <= 1.0f) {
            t = t2;
        } else {
            t = 1.0f;
        }

        f32 a = -max / (t * t);
        *pAccel = a;
        *pVel = -2.0f * t * a;
    }
    void makeAxisVerticalZX(TVec3f* pVec, const TVec3f& rAxis) {
        pVec->killElement(TVec3f(0.0f, 0.0f, 1.0f), rAxis);
        if (isNearZero(*pVec)) {
            pVec->killElement(TVec3f(1.0f, 0.0f, 0.0f), rAxis);
        }

        normalize(pVec);
    }
    void rotateVecDegree(TVec3f* pDst, const TVec3f& rSrc, const TVec3f& rAxis, f32 degree) {
        TRot3f rotation;
        rotation.identity();
        rotation.setRotate(rAxis, degree * PI_180);
        rotation.mult(rSrc, *pDst);
    }
    void rotateVecDegree(TVec3f* pDst, const TVec3f& rAxis, f32 degree) {
        rotateVecDegree(pDst, *pDst, rAxis, degree);
    }
    f32 getMaxAbsElement(const TVec3f& rVec) {
        f32* vec_arr = (f32*)(&rVec);
        return vec_arr[getMaxAbsElementIndex(rVec)];
    }
    void makeAxisFrontUp(TVec3f* pSide, TVec3f* pUp, const TVec3f& rFront, const TVec3f& rUp) {
        pSide->cross(rUp, rFront);
        normalize(pSide);
        pUp->cross(rFront, *pSide);
    }
    void makeAxisFrontSide(TVec3f* pUp, TVec3f* pSide, const TVec3f& rFront, const TVec3f& rSide) {
        pUp->cross(rFront, rSide);
        normalize(pUp);
        pSide->cross(*pUp, rFront);
    }
    f32 calcPerpendicFootToLine(TVec3f* pDst, const TVec3f& rPos, const TVec3f& rPointA, const TVec3f& rPointB) {
        TVec3f offset = rPointB - rPointA;
        f32 ratio = rPos.dot(offset) - rPointA.dot(offset);
        ratio /= offset.squared();
        offset.scale(ratio);
        pDst->set(rPointA);
        pDst->add(offset);
        return ratio;
    }
    f32 calcPerpendicFootToLineInside(TVec3f* pDst, const TVec3f& rPos, const TVec3f& rPointA, const TVec3f& rPointB) {
        TVec3f offset = rPointB - rPointA;
        f32 ratio = rPos.dot(offset) - rPointA.dot(offset);
        ratio /= offset.squared();
        ratio = JGeometry::TUtil< f32 >::clamp(ratio, 0.0f, 1.0f);
        offset.scale(ratio);
        pDst->set(rPointA);
        pDst->add(offset);
        return ratio;
    }
    f32 getScaleWithReactionValueZeroToOne(f32 x, f32 freq, f32 amplitude) {
        if (x < 0.5f) {
            return getEaseOutValue(x * 2.0f, 0.0f, 1.0f, 1.0f);
        } else {
            f32 t = (x - 0.5f) * 2.0f;
            return 1.0f + (1.0f - JMACosRadian(pi() * freq * t)) * (1.0f - t) * amplitude;
        }
    }
    void makeAxisUpFront(TVec3f* pSide, TVec3f* pFront, const TVec3f& rUp, const TVec3f& rFront) {
        pSide->cross(rUp, rFront);
        normalize(pSide);
        pFront->cross(*pSide, rUp);
    }
    void makeAxisCrossPlane(TVec3f* pVec, TVec3f* pCross, const TVec3f& rNorm) {
        makeAxisVerticalZX(pVec, rNorm);
        pCross->cross(*pVec, rNorm);
        normalizeOrZero(pCross);
    }
    void clampLength(TVec3f* pDst, const TVec3f& rSrc, f32 length) {
        if (rSrc.squared() > length * length) {
            pDst->setLength(rSrc, length);
        } else {
            pDst->set(rSrc);
        }
    }
    u32 getMaxElementIndex(const TVec3f& rVec) {
        if (rVec.x > rVec.y && rVec.x > rVec.z) {
            return 0;
        }

        if (rVec.y > rVec.z) {
            return 1;
        }

        return 2;
    }
    f32 getMaxElement(const TVec3f& rVec) {
        f32* vec_arr = (f32*)(&rVec);
        return vec_arr[getMaxElementIndex(rVec)];
    }
    void calcMomentRollBall(TVec3f* pMoment, const TVec3f& rVel, const TVec3f& rUp, f32 radius) {
        TVec3f up;
        up.set(rUp);
        if (normalizeOrZero(&up)) {
            pMoment->zero();
            return;
        }

        TVec3f moment = up.cross(rVel);
        moment /= radius;
        pMoment->set(moment);
    }
    f32 turnVecToVecRadian(TVec3f* pDst, const TVec3f& rFrom, const TVec3f& rTo, f32 angle, const TVec3f& rAxis) {
        const f32 rawAngle = JMAAcosRadian(JGeometry::TUtil< f32 >::clamp(rFrom.dot(rTo), -1.0f, 1.0f));
        f32 angleBetween = rawAngle;
        f32 angleMoved = 0.0f;
        f32 angleClamped = angleBetween;
        if (angle <= angleBetween) {
            angleMoved = angleBetween - angle;
            angleClamped = angle;
        }

        if (isNearZero(angleBetween)) {
            return 0.0f;
        }

        TQuat4f rot;
        bool isSameDir = rFrom.dot(rTo) >= 0.0f ? false : isSameDirection(rFrom, rTo);
        if (isSameDir) {
            rot.setRotate(rAxis, angleClamped);
        } else {
            rot.setRotate(rFrom, rTo, angleClamped / rawAngle);
        }

        TVec3f v = rFrom;
        rot.transform(v);
        normalizeOrZero(&v);
        if (!isNearZero(v)) {
            pDst->set(v);
        }

        return angleMoved;
    }
    f32 turnVecToVecDegree(TVec3f* pDst, const TVec3f& rFrom, const TVec3f& rTo, f32 angle, const TVec3f& rAxis) {
        return turnVecToVecRadian(pDst, rFrom, rTo, angle * (PI / 180.0f), rAxis) * _180_PI;
    }
    f32 calcDistanceXY(const TVec3f& rPos1, const TVec3f& rPos2) {
        f32 xDelta = rPos1.x - rPos2.x;
        f32 yDelta = rPos1.y - rPos2.y;
        return JGeometry::TUtil< f32 >::sqrt(xDelta * xDelta + yDelta * yDelta);
    }
    bool isNearAngleRadianHV(const TVec3f& rA, const TVec3f& rB, const TVec3f& rAxis, f32 angleH, f32 angleV) {
        if (angleV > HALF_PI_D) {
            angleV = HALF_PI;
        }

        TVec3f unitA;
        if (normalizeOrZero(rA, &unitA)) {
            return false;
        }

        TVec3f unitB;
        if (normalizeOrZero(rB, &unitB)) {
            return false;
        }

        TVec3f unitAxis;
        if (normalizeOrZero(rAxis, &unitAxis)) {
            return false;
        }

        TVec3f horizonA;
        horizonA.killElement(rA, rAxis);
        normalizeOrZero(&horizonA);

        if (horizonA.dot(unitB) < JMACosRadian(angleH)) {
            return false;
        }

        if (MR::abs(unitA.dot(unitAxis)) > MR::abs(JMASinRadian(angleV))) {
            return false;
        }

        return true;
    }
    bool isNearAngleDegreeHV(const TVec3f& rA, const TVec3f& rB, const TVec3f& rAxis, f32 angleH, f32 angleV) {
        angleV = PI_180 * angleV;
        angleH = PI_180 * angleH;

        return isNearAngleRadianHV(rA, rB, rAxis, angleH, angleV);
    }
    f32 getConvergeVibrationValue(f32 x, f32 start, f32 end, f32 dampScale, f32 rate) {
        f32 vibWeight = (x * x) * (x * x);
        f32 t1 = 1.0f - x;
        f32 convergeWeight = (t1 * t1) * (t1 * t1);
        f32 dampRatio = dampScale * t1;

        f32 vibration = JMASinRadian(PI * (x + rate * vibWeight));
        return getInterpolateValue((1.0f - convergeWeight) + vibration * dampRatio, start, end);
    }
    f32 getReduceVibrationValue(f32 x, f32 time, f32 base, f32 amplitude, f32 freq) {
        f32 vibMax = base + amplitude;

        f32 vib = JMACosRadian(x * (1.0f / freq * PI));
        f32 vibration = (amplitude * 0.5f) * (vib - 1.0f);
        if (x >= time) {
            return vibration + vibMax;
        } else {
            f32 t = x - time;
            f32 reduce = (1.0f - vibMax) * (1.0f / (time * time)) * t * t;
            reduce = vibMax + reduce;
            return reduce + vibration;
        }
    }
    void getRandomVector(TVec3f* pDst, f32 range) {
        pDst->set< f32 >(getRandom(-range, range), getRandom(-range, range), getRandom(-range, range));
    }
    f32 calcRotateY(f32 x, f32 z) {
        return MR::repeatDegree(MR::toDegree(MR::atan2(-z, x)) + 90.0f);
    }
    bool isNearAngleRadian(const TVec3f& rA, const TVec3f& rB, f32 angle) {
        if (isNearZero(rA) || isNearZero(rB)) {
            return false;
        }

        TVec3f normA;
        normalize(rA, &normA);

        TVec3f normB;
        normalize(rB, &normB);

        return normA.dot(normB) >= JMACosRadian(angle);
    }
    bool isNearAngleDegree(const TVec3f& rA, const TVec3f& rB, f32 angle) {
        return isNearAngleRadian(rA, rB, angle * (PI / 180.0f));
    }
    void makeQuatRotateDegree(TQuat4f* pQuat, const TVec3f& rRot) {
        pQuat->setEuler(rRot.x * PI_180, rRot.y * PI_180, rRot.z * PI_180);
    }
    void rotateQuatMoment(TQuat4f* pQuat, const TVec3f& rMoment) {
        TQuat4f rot;
        f32 rate;
        TVec3f axis;
        separateScalarAndDirection(&rate, &axis, rMoment);
        rot.setRotate(axis, rate);
        pQuat->mult(rot);
        pQuat->normalize();
    }
    void blendQuatUpFront(TQuat4f* pDst, const TVec3f& rUp, const TVec3f& rFront, f32 upRate, f32 frontRate) {
        blendQuatUpFront(pDst, *pDst, rUp, rFront, upRate, frontRate);
    }
    void blendQuatUpFront(TQuat4f* pDst, const TQuat4f& rSrc, const TVec3f& rUp, const TVec3f& rFront, f32 upRate, f32 frontRate) {
        TQuat4f q;
        q.set(rSrc);
        TVec3f up;
        q.getYDir(up);

        bool isUpSame = up.dot(rUp) >= 0.0f ? false : isSameDirection(up, rUp);
        if (isUpSame) {
            turnRandomVector(&up, up, 0.001f);
        }

        TQuat4f upRot;
        upRot.setRotate(up, rUp, upRate);
        q.mult(upRot);

        q.getYDir(up);
        TVec3f front;
        q.getZDir(front);

        TVec3f targetFront = rFront.killElement(up);
        MR::normalizeOrZero(&targetFront);

        bool isFrontSame = front.dot(targetFront) >= 0.0f ? false : isSameDirection(front, targetFront);
        if (isFrontSame) {
            turnRandomVector(&front, front, 0.001f);
        }

        TQuat4f frontRot;
        frontRot.setRotate(front, targetFront, frontRate);
        q.mult(frontRot);

        q.normalize();
        pDst->set(q);
    }
}

