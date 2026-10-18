#include <Game/Map/CollisionCategorizedKeeper.hpp>
#include <Game/Map/CollisionDirector.hpp>
#include <Game/Map/CollisionParts.hpp>
#include <Game/Util/CollisionPartsFilter.hpp>
#include <Game/Util/MathUtil.hpp>
#include <Game/Util/SceneUtil.hpp>
#include <Game/Scene/SceneObjHolder.hpp>
CollisionCategorizedKeeper::CollisionCategorizedKeeper(s32 category)
    : NameObj("地形コリジョンカテゴリキーパー"), mHitInfoArray(), _10(), mPartsCount(), mCategory(category), mZonesInitialized(),
      mUpdateZoneBounds(true) {
    mHitInfoArray = new HitInfo[32];
}
CollisionCategorizedKeeper::~CollisionCategorizedKeeper() {
}
void CollisionCategorizedKeeper::movement() {
    CollisionParts* pParts;

    for (CollisionZone** pZone = mZones.begin(); pZone != mZones.end(); pZone++) {
        s32 count = (*pZone)->mParts.size();

        for (int i = 0; i < count; i++) {
            pParts = (*pZone)->mParts[i];

            if (pParts->_CC) {
                if (mCategory == pParts->mKeeperIndex) {
                    pParts->updateMtx();
                }

                if (mUpdateZoneBounds) {
                    (*pZone)->calcMinMaxAndRadius();
                } else if (pParts->_D4 == 0) {
                    (*pZone)->calcMinMaxAndRadiusIfMoveOuter(pParts);
                }
            }
        }
    }

    mUpdateZoneBounds = false;
}
void CollisionCategorizedKeeper::addToZone(CollisionParts* pParts, s32 zone) {
    mZones[zone]->addParts(pParts);
    mPartsCount++;
}
void CollisionCategorizedKeeper::removeFromZone(CollisionParts* pParts, s32 zone) {
    mZones[zone]->eraseParts(pParts);
    mPartsCount--;
}
void CollisionCategorizedKeeper::addToGlobal(CollisionParts* pParts) {
    mZones[0]->addParts(pParts);
    mPartsCount++;
}
void CollisionCategorizedKeeper::removeFromGlobal(CollisionParts* pParts) {
    mZones[0]->eraseParts(pParts);
    mPartsCount--;
}
s32 CollisionCategorizedKeeper::checkStrikeBall(const TVec3f& rPosition, f32 ballRadius, bool moving, const CollisionPartsFilterBase* pPartsFilter,
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

            hitCount += pParts->checkStrikeBall(&mHitInfoArray[hitCount], 32 - hitCount, rPosition, ballRadius, moving, pTriangleFilter);

            if (hitCount >= 32) {
                _10 = hitCount;
                return hitCount;
            }
        }
    }

    _10 = hitCount;
    return hitCount;
}
bool CollisionCategorizedKeeper::isSphereOverlappingWithBox(const TVec3f& rMin, const TVec3f& rMax, const TVec3f& rCenter, f32 radius) {
    if (rCenter.x < rMin.x - radius || rMax.x + radius < rCenter.x) {
        return false;
    }

    if (rCenter.y < rMin.y - radius || rMax.y + radius < rCenter.y) {
        return false;
    }

    if (rCenter.z < rMin.z - radius || rMax.z + radius < rCenter.z) {
        return false;
    }

    return true;
}
HitInfo* CollisionCategorizedKeeper::getStrikeInfo(u32 index) {
    return &mHitInfoArray[index];
}
CollisionZone* CollisionCategorizedKeeper::getZone(int zone) {
    if (!mZonesInitialized) {
        s32 count = MR::getZoneNum();

        for (int i = 0; i < count; i++) {
            mZones.push_back(new CollisionZone(i));
        }

        mZonesInitialized = true;
    }

    return mZones[zone];
}
void CollisionZone::calcMinMaxAndRadiusIfMoveOuter(CollisionParts* pParts) {
    f32 radius = pParts->_D8;
    TVec3f position(pParts->getTrans());
    TVec3f min(mMin);
    TVec3f max(mMax);
    min.x += radius;
    min.y += radius;
    min.z += radius;
    max.x -= radius;
    max.y -= radius;
    max.z -= radius;

    if (!MR::isInRange(position.x, min.x, max.x) || !MR::isInRange(position.y, min.y, max.y) || !MR::isInRange(position.z, min.z, max.z)) {
        calcMinMaxAndRadius();
    }
}
SceneObjHolder::SceneObjHolder() {
    for (int i = 0; i < SceneObj_NumMax; i++) {
        mObj[i] = nullptr;
    }
}
NameObj* SceneObjHolder::create(int id) {
    NameObj* pObj = mObj[id];

    if (pObj != nullptr) {
        return pObj;
    }

    pObj = newEachObj(id);
    pObj->initWithoutIter();

    mObj[id] = pObj;

    return pObj;
}
NameObj* SceneObjHolder::getObj(int id) const {
    return mObj[id];
}
bool SceneObjHolder::isExist(int id) const {
    return mObj[id] != nullptr;
}
namespace MR {
bool isInRange(f32 x, f32 min, f32 max) {
        if (min > max) {
            if (x < max) {
                return false;
            }

            if (x > min) {
                return false;
            }

            return true;
        } else {
            if (x < min) {
                return false;
            }

            if (x > max) {
                return false;
            }

            return true;
        }
    }
}
#include <Game/LiveActor/Binder.hpp>
#include <Game/Util/MapUtil.hpp>
namespace { const f32 hOverlapAddValue=1.2f; }
const TVec3f Binder::bind(const TVec3f& rVelocity) {
    const bool stopAtFirstHit = _1EC._5 != 0;
    _1EC._5 = false;
    clear();

    TVec3f position(*_10);
    TVec3f movement(rVelocity);
    TVec3f result;

    if (_1EC._2 && mExCollisionParts) {
        MR::getCollisionDirector()->getCategoryKeeper(0)->addToGlobal(mExCollisionParts);
    }

    if (mOffsetVec) {
        if (_1EC._4 && _C) {
            position.x += _C[0][0] * mOffsetVec->x;
            position.y += _C[1][0] * mOffsetVec->x;
            position.z += _C[2][0] * mOffsetVec->x;
            position.x += _C[0][1] * mOffsetVec->y;
            position.y += _C[1][1] * mOffsetVec->y;
            position.z += _C[2][1] * mOffsetVec->y;
            position.x += _C[0][2] * mOffsetVec->z;
            position.y += _C[1][2] * mOffsetVec->z;
            position.z += _C[2][2] * mOffsetVec->z;
        } else {
            position.x += mOffsetVec->x;
            position.y += mOffsetVec->y;
            position.z += mOffsetVec->z;
        }
    } else if (_C) {
        position.x += _C[0][1] * mOffsetY;
        position.y += _C[1][1] * mOffsetY;
        position.z += _C[2][1] * mOffsetY;
    } else {
        position.y += mOffsetY;
    }

    TVec3f startPosition(position);
    HitInfo localPlanes[32];
    u32 capacity = _24;
    HitInfo* planes;
    if (capacity == 0) {
        planes = localPlanes;
        capacity = 32;
    } else {
        planes = mPlane;
    }

    bool hasRemainingMovement;
    u32 firstHitCount = findBindedPos(&position, &movement, &hasRemainingMovement, planes, capacity, false, stopAtFirstHit);
    if (firstHitCount == 0) {
        result = movement;
    } else {
        TVec3f firstReaction(0, 0, 0);
        TVec3f reaction(0, 0, 0);
        TVec3f slideReaction(0, 0, 0);
        obtainMomentFixReaction(planes, capacity, &firstReaction, 0);
        position.add(firstReaction);
        TVec3f previousReaction(firstReaction);

        while (!stopAtFirstHit && hasRemainingMovement == true) {
            TVec3f remaining(rVelocity - movement);
            moveAlongHittedPlanes(&movement, &position, &remaining, rVelocity, previousReaction, planes, capacity, &hasRemainingMovement);
            obtainMomentFixReaction(planes, capacity, &reaction, firstHitCount);
            position.add(reaction);
            previousReaction.set(reaction);
            slideReaction.add(reaction);
            hasRemainingMovement = false;
        }

        storeContactPlane(planes, capacity);
        if (_1EC._0) {
            moveWithCollisionParts(&position, &movement);
        }

        mFixReactionVector = firstReaction + slideReaction;
        result = position - startPosition;
    }

    if (_1EC._2 && mExCollisionParts) {
        MR::getCollisionDirector()->getCategoryKeeper(0)->removeFromGlobal(mExCollisionParts);
    }

    return result;
}
void Binder::moveAlongHittedPlanes(TVec3f* pMovement, TVec3f* pPosition, TVec3f* pRemaining, const TVec3f& rVelocity, const TVec3f& rReaction,
                                   HitInfo* pPlanes, u32 capacity, bool* pHasRemainingMovement) {
    TVec3f normal(rReaction);
    MR::normalizeOrZero(&normal);
    f32 dot = pRemaining->dot(normal);
    if (dot < 0.0f) {
        pRemaining->sub(normal * dot);
    }

    if (rVelocity.dot(*pRemaining) < 0.0f) {
        *pHasRemainingMovement = false;
    } else {
        findBindedPos(pPosition, pRemaining, pHasRemainingMovement, pPlanes, capacity, true, false);
        pMovement->add(*pRemaining);
    }
}
u32 Binder::findBindedPos(TVec3f* pPosition, TVec3f* pMovement, bool* pHasRemainingMovement, HitInfo* pPlanes, u32 capacity, bool skipInitial,
                          bool stopAtFirstHit) {
    s32 steps = static_cast< s32 >((1.0f / 35.0f) * pMovement->length()) + 1;
    TVec3f step(*pMovement);
    if (steps > 1) {
        step /= steps;
    }

    pMovement->zero();
    for (s32 i = 0; i <= steps; i++) {
        if (i != 0) {
            pPosition->add(step);
            pMovement->add(step);
        } else if (_1EC._3 || skipInitial) {
            continue;
        }

        s32 hitCount;
        if (_1EC._1) {
            hitCount = Collision::checkStrikeBallToMapWithMovingReaction(*pPosition, mRadius, mCollisionPartsFilter, mTriangleFilter);
        } else {
            hitCount = Collision::checkStrikeBallToMap(*pPosition, mRadius, mCollisionPartsFilter, mTriangleFilter);
        }

        if (hitCount != 0) {
            if (i == steps) {
                *pHasRemainingMovement = false;
            } else {
                *pHasRemainingMovement = true;
            }

            return storeCurrentHitInfo(pPlanes, capacity, stopAtFirstHit);
        }
    }

    *pHasRemainingMovement = false;
    return 0;
}
bool Binder::moveWithCollisionParts(TVec3f* pPosition, TVec3f* pMovement) {
    if (_C8 < 0.0f) {
        return false;
    }

    if (!mGroundInfo.mParentTriangle.isHostMoved()) {
        return false;
    }

    TVec3f force;
    mGroundInfo.mParentTriangle.calcForceMovePower(&force, *pPosition);
    pMovement->add(force);
    pPosition->add(force);
    return true;
}
u32 Binder::storeCurrentHitInfo(HitInfo* pPlanes, u32 capacity, bool stopAtFirstHit) {
    u32 hitCount = Collision::getStrikeInfoNumMap();
    u32 count = 0;
    for (u32 i = 0; i < hitCount; i++) {
        if (capacity <= count + mPlaneNum) {
            u32 stored = capacity - mPlaneNum;
            mPlaneNum = capacity;
            return stored;
        }

        pPlanes[count + mPlaneNum] = *Collision::getStrikeInfoMap(i);
        if (!stopAtFirstHit) {
            pPlanes[i + mPlaneNum]._60 += ::hOverlapAddValue;
        }

        count++;
    }

    mPlaneNum += count;
    return count;
}
bool Triangle::isHostMoved() const {
    return mParts->_D4 == 0;
}
void Triangle::calcForceMovePower(TVec3f* a1, const TVec3f& a2) const {
    mParts->calcForceMovePower(a1, a2);
}








#include <Game/Camera/CameraPolygonCodeUtil.hpp>
#include <Game/Map/KCollision.hpp>
#include <Game/Map/CollisionCode.hpp>
bool KCollisionServer::calcFarthestVertexDistance() {
    s32 triCount = getTriangleNum();
    f32 maxDistance = 0.0f;
    KC_PrismData* prism;
    bool result = true;

    for (u32 i = 0; i < (u32)triCount; i++) {
        prism = &mFile->mPrisms[i + 1];
        JMapInfoIter iter = getAttributes(i);

        if (!iter.isValid()) {
            result = false;
        } else {
            MR::registerCameraCode(MR::getCollisionDirector()->mCode->getCameraID(iter));
        }

        if (isNearParallelNormal(prism)) {
            prism->mHeight = -MR::abs(prism->mHeight);
        } else {
            for (s32 j = 0; j < 3; j++) {
                TVec3f pos = getPos(prism, j);
                f32 distSq = pos.squared();

                if (maxDistance < distSq) {
                    maxDistance = distSq;
                }
            }
        }
    }

    mMaxVertexDistance = MR::sqrt(maxDistance);
    return result;
}
bool KCollisionServer::isNearParallelNormal(const KC_PrismData* pPrism) const {
    TVec3f edge0 = mFile->mNorms[pPrism->mEdgeIndices[0]];
    TVec3f edge1 = mFile->mNorms[pPrism->mEdgeIndices[1]];
    TVec3f edge2 = mFile->mNorms[pPrism->mEdgeIndices[2]];

    bool isNear = false;

    if (MR::isSameDirection(edge0, edge1) || MR::isSameDirection(edge0, edge2) || MR::isSameDirection(edge1, edge2)) {
        isNear = true;
    }

    return isNear;
}
namespace MR {
bool normalizeOrZero(TVec3f* pVec) {
        if (isNearZero(*pVec)) {
            pVec->zero();
            return true;
        }

        normalize(pVec);
        return false;
    }
}
namespace {
u32 getStrikeInfoNumCategory(s32 category) {
        return MR::getCollisionDirector()->getCategoryKeeper(category)->_10;
    }
}
namespace Collision {
s32 checkStrikeBallToMap(const TVec3f& rCenter, f32 radius, const CollisionPartsFilterBase* pPartsFilter,
                             const TriangleFilterBase* pTriangleFilter) {
        return MR::getCollisionDirector()->getCategoryKeeper(0)->checkStrikeBall(rCenter, radius, false, pPartsFilter, pTriangleFilter);
    }
s32 checkStrikeBallToMapWithMovingReaction(const TVec3f& rCenter, f32 radius, const CollisionPartsFilterBase* pPartsFilter,
                                               const TriangleFilterBase* pTriangleFilter) {
        return MR::getCollisionDirector()->getCategoryKeeper(0)->checkStrikeBall(rCenter, radius, true, pPartsFilter, pTriangleFilter);
    }
const HitInfo* getStrikeInfoMap(u32 index) {
        return MR::getCollisionDirector()->getCategoryKeeper(0)->getStrikeInfo(index);
    }
u32 getStrikeInfoNumMap() {
        return ::getStrikeInfoNumCategory(0);
    }
}
