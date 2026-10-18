#include <Game/Util/MathUtil.hpp>
#include <cmath>
namespace MR {
    bool checkHitSegmentSphere(const TVec3f& rSpherePos, const TVec3f& rPointA, const TVec3f& rPointB, f32 radius, TVec3f* pDir) {
        TVec3f pos = rSpherePos;
        TVec3f segment = rPointB;
        pos -= rPointA;
        segment -= rPointA;

        f32 dot = pos.dot(segment);
        f32 radSquared = radius * radius;
        if (dot < 0.0f) {
            if (rPointA.squared(rSpherePos) < radSquared) {
                if (pDir != nullptr) {
                    pDir->set(rSpherePos - rPointA);
                    MR::normalizeOrZero(pDir);
                }

                return true;
            }
        } else if (segment.squared() < dot) {
            if (rPointB.squared(rSpherePos) < radSquared) {
                if (pDir != nullptr) {
                    pDir->set(rSpherePos - rPointB);
                    MR::normalizeOrZero(pDir);
                }

                return true;
            }
        } else {
            TVec3f offset = segment;
            offset.scale(dot / segment.squared());
            offset.sub(pos);
            if (offset.squared() <= radSquared) {
                if (pDir != nullptr) {
                    pDir->set(-offset);
                    MR::normalizeOrZero(pDir);
                }

                return true;
            }
        }

        return false;
    }
    bool checkHitSemilinePlane(TVec3f* pHitPos, const TVec3f& rPos, const TVec3f& rDir, const TVec3f& rPlanePos, const TVec3f& rPlaneNorm) {
        f32 dot = rDir.dot(rPlaneNorm);
        TVec3f offset = rPlanePos - rPos;
        if (0.0f < dot) {
            return false;
        }

        f32 ratio = rPlaneNorm.dot(offset) / dot;
        pHitPos->set(rPos);
        pHitPos->add(rDir * ratio);

        return true;
    }
    bool normalizeOrZero(TVec2f* pVec) {
        if (isNearZero(*pVec)) {
            pVec->zero();
            return true;
        }

        normalize(pVec);
        return false;
    }
    void normalize(const TVec2f& rSrc, TVec2f* pDst) {
        *pDst = rSrc;
        normalize(pDst);
    }
    void separateScalarAndDirection(f32* pScalar, TVec2f* pDir, const TVec2f& rVec) {
        *pScalar = rVec.length();

        if (isNearZero(rVec)) {
            pDir->zero();
        } else {
            normalize(rVec, pDir);
        }
    }
    bool isNearZero(const TVec2f& rVec, f32 tolerance) {
        if (rVec.x > tolerance) {
            return false;
        }

        if (rVec.x < -tolerance) {
            return false;
        }

        if (rVec.y > tolerance) {
            return false;
        }

        if (rVec.y < -tolerance) {
            return false;
        }

        return true;
    }
    f32 convergeRadian(f32 angle, f32 target, f32 speed) {
        f32 convergeTarget = target;
        if ((target + pi2()) - angle < pi()) {
            convergeTarget = target + pi2();
        } else if (angle - (target - pi2()) < pi()) {
            convergeTarget = target - pi2();
        }

        if (angle < convergeTarget) {
            angle += speed;
            if (angle > convergeTarget) {
                angle = convergeTarget;
            }
        } else {
            angle -= speed;
            if (angle < convergeTarget) {
                angle = convergeTarget;
            }
        }

        return repeat(angle, 0.0f, pi2());
    }
    f32 normalizeAngleAbs(f32 angle) {
        while (true) {
            if (angle < 0.0f) {
                angle += TWO_PI;
            } else if (angle > TWO_PI) {
                angle -= TWO_PI;
            } else {
                return angle;
            }
        }
    }
    bool isAngleBetween(f32 angle, f32 min, f32 max) {
        f32 a1_n = normalizeAngleAbs(angle);
        f32 a2_n = normalizeAngleAbs(min);
        f32 a3_n = normalizeAngleAbs(max);

        if (a3_n > a2_n) {
            f32 val = a3_n;
            a3_n = a2_n;
            a2_n = val;
        }

        bool res = a1_n >= a3_n && a1_n <= a2_n;

        if (a2_n - a3_n > PI) {
            res = !res;
        }

        return res;
    }
    f32 blendAngle(f32 angleA, f32 angleB, f32 rate) {
        angleA = normalizeAngleAbs(angleA);
        angleB = normalizeAngleAbs(angleB);

        if (!isAngleBetween((angleA + angleB) / 2.0f, angleA, angleB)) {
            if (angleA < angleB) {
                angleA += TWO_PI;
            } else {
                angleB += TWO_PI;
            }
        }

        return normalizeAngleAbs((1.0f - rate) * angleA + rate * angleB);
    }
    f32 mod(f32 x, f32 y) {
        return fmod(x, y);
    }
    f32 normalizeAbs(f32 x, f32 min, f32 max) {
        if (x >= 0.0f) {
            return normalize(x, min, max);
        } else {
            return -normalize(-x, min, max);
        }
    }
}

