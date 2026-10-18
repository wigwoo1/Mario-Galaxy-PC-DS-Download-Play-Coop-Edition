#include <Game/Util/MathUtil.hpp>
namespace MR {
bool isSameDirection(const TVec3f& rVec1, const TVec3f& rVec2, f32 tolerance) {
        if (MR::abs(rVec1.y * rVec2.z - rVec1.z * rVec2.y) > tolerance) {
            return false;
        }

        if (MR::abs(rVec1.z * rVec2.x - rVec1.x * rVec2.z) > tolerance) {
            return false;
        }

        if (MR::abs(rVec1.x * rVec2.y - rVec1.y * rVec2.x) > tolerance) {
            return false;
        }

        return true;
    }
bool isNearZero(f32 x, f32 tolerance) {
        if (x < 0.0f) {
            x = -x;
        }

        if (x < tolerance) {
            return true;
        }

        return false;
    }
bool isNearZero(const TVec3f& rVec, f32 tolerance) {
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

        if (rVec.z > tolerance) {
            return false;
        }

        if (rVec.z < -tolerance) {
            return false;
        }

        return true;
    }
void normalize(TVec3f* pVec) {
        pVec->normalize();
    }
void normalize(const TVec3f& rSrc, TVec3f* pDst) {
        pDst->set(rSrc);
        normalize(pDst);
    }
void separateScalarAndDirection(f32* pScalar, TVec3f* pDir, const TVec3f& rVec) {
        *pScalar = rVec.length();

        if (isNearZero(rVec)) {
            pDir->zero();
        } else {
            normalize(rVec, pDir);
        }
    }
}