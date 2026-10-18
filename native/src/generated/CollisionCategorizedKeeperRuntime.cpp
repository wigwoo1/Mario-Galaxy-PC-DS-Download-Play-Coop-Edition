#include "Game/Map/CollisionCategorizedKeeper.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Map/CollisionDirector.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util/CollisionPartsFilter.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/SceneUtil.hpp"













s32 CollisionCategorizedKeeper::checkStrikePoint(const TVec3f& rPosition, HitInfo* pHitInfo) {
    MR::getCollisionDirector();
    _10 = 0;

    for (CollisionZone** pZone = mZones.begin(); pZone != mZones.end(); pZone++) {
        if (pZone != mZones.begin()) {
            if (!isSphereOverlappingWithBox((*pZone)->mMin, (*pZone)->mMax, rPosition, 0.0f)) {
                continue;
            }

            f32 radius = (*pZone)->mRadius;
            TVec3f offset((*pZone)->mCenter);
            offset -= rPosition;

            if (radius * radius < offset.squared()) {
                continue;
            }
        }

        int count = (*pZone)->mParts.size();
        CollisionParts* pParts;

        for (int i = 0; i < count; i++) {
            pParts = (*pZone)->mParts[i];

            if (!pParts->_CC) {
                continue;
            }

            f32 radius = pParts->_D8;
            TVec3f offset;
            offset.x = MR::abs(pParts->getTrans().x - rPosition.x);

            if (radius < offset.x) {
                continue;
            }

            offset.y = MR::abs(pParts->getTrans().y - rPosition.y);

            if (radius < offset.y) {
                continue;
            }

            offset.z = MR::abs(pParts->getTrans().z - rPosition.z);

            if (radius < offset.z) {
                continue;
            }

            if (offset.squared() > radius * radius) {
                continue;
            }

            if (pParts->checkStrikePoint(pHitInfo, rPosition)) {
                _10 = 1;
                return 1;
            }
        }
    }

    return _10;
}



s32 CollisionCategorizedKeeper::checkStrikeBallWithThickness(const TVec3f& rPosition, f32 ballRadius, f32 thickness,
                                                             const CollisionPartsFilterBase* pPartsFilter,
                                                             const TriangleFilterBase* pTriangleFilter) {
    MR::getCollisionDirector();
    _10 = 0;
    s32 hitCount = 0;

    for (CollisionZone** pZone = mZones.begin(); pZone != mZones.end(); pZone++) {
        if (pZone != mZones.begin()) {
            if (!isSphereOverlappingWithBox((*pZone)->mMin, (*pZone)->mMax, rPosition, ballRadius)) {
                continue;
            }

            f32 radius = ballRadius + (*pZone)->mRadius;
            TVec3f offset((*pZone)->mCenter);
            offset -= rPosition;

            if (radius * radius < offset.squared()) {
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

            f32 radius = ballRadius + pParts->_D8;
            TVec3f offset;
            offset.x = MR::abs(pParts->getTrans().x - rPosition.x);

            if (radius < offset.x) {
                continue;
            }

            offset.y = MR::abs(pParts->getTrans().y - rPosition.y);

            if (radius < offset.y) {
                continue;
            }

            offset.z = MR::abs(pParts->getTrans().z - rPosition.z);

            if (radius < offset.z) {
                continue;
            }

            if (offset.squared() > radius * radius) {
                continue;
            }

            hitCount +=
                pParts->checkStrikeBallWithThickness(&mHitInfoArray[hitCount], 32 - hitCount, rPosition, ballRadius, thickness, pTriangleFilter);

            if (hitCount >= 32) {
                _10 = hitCount;
                return hitCount;
            }
        }
    }

    _10 = hitCount;
    return hitCount;
}

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

u32 CollisionCategorizedKeeper::createAreaPolygonList(Triangle* pTriangles, u32 maxTriangles, const TVec3f& rStart, const TVec3f& rEnd) {
    MR::getCollisionDirector();
    u32 triangleCount = 0;
    TVec3f min;
    TVec3f max;

    if (rStart.x < rEnd.x) {
        min.x = rStart.x;
        max.x = rEnd.x;
    } else {
        min.x = rEnd.x;
        max.x = rStart.x;
    }

    if (rStart.y < rEnd.y) {
        min.y = rStart.y;
        max.y = rEnd.y;
    } else {
        min.y = rEnd.y;
        max.y = rStart.y;
    }

    if (rStart.z < rEnd.z) {
        min.z = rStart.z;
        max.z = rEnd.z;
    } else {
        min.z = rEnd.z;
        max.z = rStart.z;
    }

    for (CollisionZone** pZone = mZones.begin(); pZone != mZones.end(); pZone++) {
        if (pZone != mZones.begin() && !isSphereOverlappingWithBox(min, max, (*pZone)->mCenter, (*pZone)->mRadius)) {
            continue;
        }

        s32 count = (*pZone)->mParts.size();
        CollisionParts* pParts;

        for (int i = 0; i < count; i++) {
            pParts = (*pZone)->mParts[i];

            if (!pParts->_CC) {
                continue;
            }

            f32 radius = pParts->_D8;
            TVec3f position(pParts->getTrans());

            if (!isSphereOverlappingWithBox(min, max, position, radius)) {
                continue;
            }

            triangleCount += pParts->createAreaPolygonList(pTriangles + triangleCount, maxTriangles - triangleCount, rStart, rEnd);

            if (maxTriangles <= triangleCount) {
                return triangleCount;
            }
        }
    }

    return triangleCount;
}

u32 CollisionCategorizedKeeper::createAreaPolygonListArray(Triangle* pTriangles, u32 maxTriangles, TVec3f* pVertices, u32 vertexCount) {
    MR::getCollisionDirector();
    u32 triangleCount = 0;
    TVec3f min;
    TVec3f max;
    MR::createBoundingBox(pVertices, vertexCount, &min, &max);

    for (CollisionZone** pZone = mZones.begin(); pZone != mZones.end(); pZone++) {
        if (pZone != mZones.begin() && !isSphereOverlappingWithBox(min, max, (*pZone)->mCenter, (*pZone)->mRadius)) {
            continue;
        }

        s32 count = (*pZone)->mParts.size();
        CollisionParts* pParts;

        for (int i = 0; i < count; i++) {
            pParts = (*pZone)->mParts[i];

            if (!pParts->_CC) {
                continue;
            }

            f32 radius = pParts->_D8;
            TVec3f position(pParts->getTrans());

            if (!isSphereOverlappingWithBox(min, max, position, radius)) {
                continue;
            }

            triangleCount += pParts->createAreaPolygonListArray(pTriangles + triangleCount, maxTriangles - triangleCount, pVertices, vertexCount);

            if (maxTriangles <= triangleCount) {
                return triangleCount;
            }
        }
    }

    return triangleCount;
}



bool CollisionCategorizedKeeper::searchSameHostParts(CollisionParts** pResult, CollisionParts* pParts) const {
    for (CollisionZone* const* pZone = mZones.begin(); pZone != mZones.end(); pZone++) {
        s32 count = (*pZone)->mParts.size();

        for (int i = 0; i < count; i++) {
            CollisionParts* pCandidate = (*pZone)->mParts[i];

            if (pCandidate->mHitSensor->mHost == pParts->mHitSensor->mHost) {
                *pResult = pCandidate;
                return true;
            }
        }
    }

    return false;
}




















