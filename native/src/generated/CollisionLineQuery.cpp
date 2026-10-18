#include "Game/Map/CollisionCategorizedKeeper.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Map/CollisionDirector.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util/CollisionPartsFilter.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/SceneUtil.hpp"

s32 CollisionCategorizedKeeper::checkStrikeLine(const TVec3f& rStart, const TVec3f& rDirection, s32 maxHits,
                                                const CollisionPartsFilterBase* pPartsFilter, const TriangleFilterBase* pTriangleFilter) {
    MR::getCollisionDirector();

    if (maxHits == 0) {
        maxHits = 32;
    }

    _10 = 0;
    s32 hitCount = 0;
    TVec3f min(rStart);
    TVec3f max(rStart);

    if (rDirection.x < 0.0f) {
        min.x += rDirection.x;
    } else {
        max.x += rDirection.x;
    }

    if (rDirection.y < 0.0f) {
        min.y += rDirection.y;
    } else {
        max.y += rDirection.y;
    }

    if (rDirection.z < 0.0f) {
        min.z += rDirection.z;
    } else {
        max.z += rDirection.z;
    }

    for (CollisionZone** pZone = mZones.begin(); pZone != mZones.end(); pZone++) {
        if (pZone != mZones.begin()) {
            TVec3f position((*pZone)->mCenter);
            f32 radius = (*pZone)->mRadius;

            if (!isSphereOverlappingWithBox(min, max, position, radius)) {
                continue;
            }

            if (!MR::checkHitSegmentSphere(position, rStart, rStart + rDirection, radius, nullptr)) {
                continue;
            }
        }

        s32 count = (*pZone)->mParts.size();
        CollisionParts* pParts;

        for (int i = 0; i < count; i++) {
            pParts = (*pZone)->mParts[i];

            if (!pParts->_CC) {
                continue;
            }

            if (pPartsFilter != nullptr && pPartsFilter->isInvalidParts(pParts)) {
                continue;
            }

            TVec3f position(pParts->getTrans());
            f32 radius = pParts->_D8;

            if (!isSphereOverlappingWithBox(min, max, position, radius)) {
                continue;
            }

            if (!MR::checkHitSegmentSphere(position, rStart, rStart + rDirection, radius, nullptr)) {
                continue;
            }

            hitCount += pParts->checkStrikeLine(&mHitInfoArray[hitCount], maxHits - hitCount, rStart, rDirection, pTriangleFilter);

            if (maxHits <= hitCount) {
                _10 = hitCount;
                return hitCount;
            }
        }
    }

    _10 = hitCount;
    return hitCount;
}
