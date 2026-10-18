#include <revolution/gx/GXLighting.h>
#include <Game/Util/MathUtil.hpp>
namespace MR {
    void blendColor(GXColor* pDst, const GXColor& rStart, const GXColor& rEnd, f32 t) {
        pDst->r = getInterpolateValue(t, rStart.r, rEnd.r);
        pDst->g = getInterpolateValue(t, rStart.g, rEnd.g);
        pDst->b = getInterpolateValue(t, rStart.b, rEnd.b);
        pDst->a = getInterpolateValue(t, rStart.a, rEnd.a);
    }
    void blendVec(Vec* pDst, const Vec& rStart, const Vec& rEnd, f32 t) {
        pDst->x = getInterpolateValue(t, rStart.x, rEnd.x);
        pDst->y = getInterpolateValue(t, rStart.y, rEnd.y);
        pDst->z = getInterpolateValue(t, rStart.z, rEnd.z);
    }
}
