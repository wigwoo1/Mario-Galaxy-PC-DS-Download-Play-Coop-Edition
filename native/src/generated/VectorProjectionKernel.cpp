#include <Game/Util/MathUtil.hpp>
#include <cmath>
namespace MR {static f32 nativeKillElement(const TVec3f& source,const TVec3f& direction,TVec3f* output){const f32 x=source.x,y=source.y,z=source.z;f32 dot=x*direction.x+y*direction.y+z*direction.z;output->set(std::fma(-dot,direction.x,x),std::fma(-dot,direction.y,y),std::fma(-dot,direction.z,z));return dot;}
#ifndef GALAXY_SHARED_RUNTIME_MATH
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
#endif

    f32 vecKillElement(const TVec3f& rSrc, const TVec3f& rKillDir, TVec3f* pDst) {
        if (isNearZero(rKillDir)) {
            *pDst = rSrc;

            return 0.0f;
        }

        return nativeKillElement(rSrc, rKillDir, pDst);
    }
}

