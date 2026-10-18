#include <Game/Util/MtxUtil.hpp>
#include <Game/Util/MathUtil.hpp>
#include <cstring>
namespace MR {
void setMtxTrans(MtxPtr pMtx, f32 x, f32 y, f32 z) {
        pMtx[0][3] = x;
        pMtx[1][3] = y;
        pMtx[2][3] = z;
    }
void extractMtxTrans(MtxPtr pMtx, TVec3f* pOut) {
        pOut->x = pMtx[0][3];
        pOut->y = pMtx[1][3];
        pOut->z = pMtx[2][3];
    }

void vecBlend(const TVec3f& rFrom, const TVec3f& rTo, TVec3f* pDst, f32 rate) {
        PSvecBlend(&rFrom, &rTo, pDst, 1.0f - rate, rate);
    }
}

