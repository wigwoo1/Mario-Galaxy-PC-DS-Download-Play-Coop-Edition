#include <Game/Util/MathUtil.hpp>
namespace MR {
    bool normalizeOrZero(TVec3f* pVec) {
        if (isNearZero(*pVec)) {
            pVec->zero();
            return true;
        }

        normalize(pVec);
        return false;
    }
    void extractMtxTrans(MtxPtr pMtx, TVec3f* pOut) {
        pOut->x = pMtx[0][3];
        pOut->y = pMtx[1][3];
        pOut->z = pMtx[2][3];
    }
}
