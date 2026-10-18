#include <Game/Util/MathUtil.hpp>
namespace MR {bool normalizeOrZero(TVec3f* pVec) {
        if (isNearZero(*pVec)) {
            pVec->zero();
            return true;
        }

        normalize(pVec);
        return false;
    }}
