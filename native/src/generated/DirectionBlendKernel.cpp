#include <Game/Util/MathUtil.hpp>
#include <JSystem/JMath/JMATrigonometric.hpp>
namespace { static f32* gAcosTable; }
namespace MR {
    void initAcosTable() {
        ::gAcosTable = new f32[256];

        for (u32 i = 0; i < 256; i++) {
            f64 x = (i / 255.0) * (1.0 - 0.98) + 0.98;

            if (x > 1.0f) {
                x = 1.0f;
            }

            ::gAcosTable[i] = ::acos(x);
        }
    }
    f32 acosEx(f32 x) {
        if (MR::abs(x) < 0.98f) {
            return JMAAcosRadian(x);
        } else if (x < 0.0f) {
            u32 index = static_cast< u32 >((-x - 0.98f) * 50.0f * 255.0f);
            f32 acos = ::gAcosTable[index];

            return pi() - acos;
        } else {
            u32 index = static_cast< u32 >((x - 0.98f) * 50.0f * 255.0f);

            return ::gAcosTable[index];
        }
    }
    f32 diffAngleAbs(const TVec3f& rA, const TVec3f& rB) {
        f32 length1 = rA.length();
        f32 length2 = rB.length();
        f32 x = rA.dot(rB) / (length1 * length2);

        if (x >= 1.0f) {
            return 0.0f;
        } else if (x <= -1.0f) {
            return PI;
        } else {
            return acosEx(x);
        }
    }
    f32 diffAngleSigned(const TVec3f& rA, const TVec3f& rB, const TVec3f& rAxis) {
        f32 angleDiff = diffAngleAbs(rA, rB);

        TVec3f v = rA.cross(rAxis);

        if (v.dot(rB) >= 0.0f) {
            return angleDiff;
        } else {
            return -angleDiff;
        }
    }
    void vecBlendNormal(const TVec3f& rFrom, const TVec3f& rTo, TVec3f* pVec, f32 rate) {
        TVec3f vA;
        if (isNearZero(rFrom)) {
            vA.zero();
        } else {
            vA.set(rFrom);
            PSVECNormalize(vA, vA);
            PSVECScale(vA, vA, 1.0f - rate);
        }

        TVec3f vB;
        if (isNearZero(rTo)) {
            vB.zero();
        } else {
            vB.set(rTo);
            PSVECNormalize(vB, vB);
            PSVECScale(vB, vB, rate);
        }

        pVec->add(vA, vB);
    }
    bool vecBlendSphere(const TVec3f& rFrom, const TVec3f& rTo, TVec3f* pVec, f32 rate) {
        f32 lenFrom = rFrom.length();
        f32 lenTo = rTo.length();
        f32 diffAngle;
        if (lenFrom == 0.0f || lenTo == 0.0f) {
            diffAngle = 0.0f;
        } else {
            diffAngle = diffAngleAbs(rFrom, rTo);
        }

        if (diffAngle < 0.1f) {
            vecBlendNormal(rFrom, rTo, pVec, rate);
            pVec->setLength(lenFrom * (1.0f - rate) + lenTo * rate);
            return true;
        }

        if (diffAngle == 0.0f) {
            *pVec = rFrom;
            pVec->setLength(lenFrom * (1.0f - rate) + lenTo * rate);
            return false;
        }

        if (diffAngle == pi()) {
            return false;
        }

        *pVec = (rFrom * JMASinRadian(diffAngle * (1.0f - rate)) + rTo * JMASinRadian(diffAngle * rate)) / JMASinRadian(diffAngle);
        pVec->setLength(lenFrom * (1.0f - rate) + lenTo * rate);

        return true;
    }
}
