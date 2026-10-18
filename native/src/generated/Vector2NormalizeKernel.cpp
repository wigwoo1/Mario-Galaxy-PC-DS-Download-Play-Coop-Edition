#include <Game/Util/MathUtil.hpp>
namespace MR {void normalize(TVec2f* pVec) {
        TVec3f temp(pVec->x, pVec->y, 0.0f);
        normalize(&temp);
        pVec->x = temp.x;
        pVec->y = temp.y;
    }}
