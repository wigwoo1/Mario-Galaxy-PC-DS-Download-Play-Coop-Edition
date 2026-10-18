#include <Game/Util/MathUtil.hpp>
namespace MR {
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
}
