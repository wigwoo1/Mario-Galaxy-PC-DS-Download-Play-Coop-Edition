#include "Game/Util/MapUtil.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Map/CollisionCategorizedKeeper.hpp"
#include "Game/Map/CollisionCode.hpp"
#include "Game/Map/CollisionDirector.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Util/CollisionPartsFilter.hpp"
#include "Game/Util/GravityUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/TriangleFilter.hpp"

static HitInfo mSortBuffer[32];
static u32 mSortCount;
namespace {
    const HitInfo* getStrikeInfoCategory(s32 category, u32 index) {
        return MR::getCollisionDirector()->getCategoryKeeper(category)->getStrikeInfo(index);
    }
    bool getFirstPolyOnLineCategory(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection,
                                    const TriangleFilterBase* pTriangleFilter, const CollisionPartsFilterBase* pPartsFilter, s32 category) {
        u32 count = MR::getCollisionDirector()->getCategoryKeeper(category)->checkStrikeLine(rStart, rDirection, 0, pPartsFilter, nullptr);
        const HitInfo* pHit;
        if (count == 0) {
            return false;
        }

        f32 minDistance = 1000000.0f;
        s32 nearest = -1;
        for (u32 i = 0; i < count; i++) {
            pHit = getStrikeInfoCategory(category, i);
            if (pTriangleFilter && pTriangleFilter->isInvalidTriangle(&pHit->mParentTriangle)) {
                continue;
            }

            if (minDistance > pHit->_60) {
                nearest = i;
                minDistance = pHit->_60;
            }
        }

        if (nearest == -1) {
            return false;
        }

        pHit = getStrikeInfoCategory(category, nearest);
        if (pPos) {
            *pPos = pHit->mHitPos;
        }

        if (pTriangle) {
            *pTriangle = pHit->mParentTriangle;
        }

        return true;
    }
}
namespace MR {
    bool getFirstPolyOnLineToMap(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection,
                                 const CollisionPartsFilterBase* pPartsFilter, const TriangleFilterBase* pTriangleFilter) {
        return ::getFirstPolyOnLineCategory(pPos, pTriangle, rStart, rDirection, pTriangleFilter, pPartsFilter, 0);
    }
    bool getFirstPolyOnLineToWaterSurface(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection,
                                          const CollisionPartsFilterBase* pPartsFilter, const TriangleFilterBase* pTriangleFilter) {
        return ::getFirstPolyOnLineCategory(pPos, pTriangle, rStart, rDirection, pTriangleFilter, pPartsFilter, 2);
    }
    bool getFirstPolyOnLineToMap(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection) {
        return ::getFirstPolyOnLineCategory(pPos, pTriangle, rStart, rDirection, nullptr, nullptr, 0);
    }
    bool getFirstPolyOnLineToWaterSurface(TVec3f* pPos, Triangle* pTriangle, const TVec3f& rStart, const TVec3f& rDirection) {
        return ::getFirstPolyOnLineCategory(pPos, pTriangle, rStart, rDirection, nullptr, nullptr, 2);
    }
    const Triangle* getSortedPoly(u32 sortIndex) {
        if (mSortCount <= sortIndex) {
            return nullptr;
        }

        return &mSortBuffer[sortIndex].mParentTriangle;
    }
    const Triangle* getCameraPolyFast(const TVec3f& rStart, const TVec3f& rDirection, const HitSensor* pSensor) {
        Triangle triangle;
        f32 remaining = rDirection.length();
        TVec3f direction(rDirection);
        direction.normalize();
        TVec3f start(rStart);
        TVec3f step(direction * 5000.0f);
        do {
            f32 length = 5000.0f;
            if (remaining < length) {
                length = remaining;
                step = direction * remaining;
            }

            s32 count = getNearPolyOnLineSort(start, start, step, pSensor);
            while (count != 0) {
                return getSortedPoly(0);
            }

            remaining -= length;
            start += step;
        } while (!isNearZero(remaining));

        return nullptr;
    }
    const TVec3f* getNormal(const Triangle* pTriangle) {
        return pTriangle->getNormal(0);
    }
    bool isWaterPolygon(const Triangle* pTriangle) {
        const char* pFloorCodeString = getFloorCodeString(pTriangle);

        if (pFloorCodeString != nullptr) {
            if (strcmp(pFloorCodeString, "Water") == 0) {
                return true;
            }

            if (strcmp(pFloorCodeString, "Shallow") == 0) {
                return true;
            }
        }

        return false;
    }
    u32 getNearPolyOnLineSort(const TVec3f& rOrigin, const TVec3f& rStart, const TVec3f& rDirection, const HitSensor* pSensor) {
        u32 count = getCollisionDirector()->getCategoryKeeper(0)->checkStrikeLine(rStart, rDirection, 0, nullptr, nullptr);
        if (count == 0) {
            return 0;
        }

        const HitInfo* hits[32];
        u32 excluded = 0;
        for (u32 i = 0; i < count; i++) {
            hits[i] = getCollisionDirector()->getCategoryKeeper(0)->getStrikeInfo(i);
            if (pSensor && hits[i]->mParentTriangle.getSensor() == pSensor) {
                hits[i] = nullptr;
                excluded++;
            }
        }

        mSortCount = count - excluded;
        if (mSortCount >= 32) {
            mSortCount = 32;
        }

        for (u32 i = 0; i < mSortCount; i++) {
            f32 minDistance = 1000000.0f;
            u32 nearest = 0;
            for (u32 j = 0; j < count; j++) {
                if (hits[j]) {
                    const HitInfo* pHit = getCollisionDirector()->getCategoryKeeper(0)->getStrikeInfo(j);
                    TVec3f offset(rOrigin);
                    offset.sub(pHit->mHitPos);
                    f32 distance = offset.length();
                    if (minDistance > distance) {
                        nearest = j;
                        minDistance = distance;
                    }
                }
            }

            mSortBuffer[i] = *getCollisionDirector()->getCategoryKeeper(0)->getStrikeInfo(nearest);
            hits[nearest] = nullptr;
        }

        return mSortCount;
    }
    bool getSortedPoly(TVec3f* pDst, Triangle* pTriangle, u32 sortIndex) {
        if (mSortCount <= sortIndex) {
            return false;
        }

        const HitInfo& rHit = mSortBuffer[sortIndex];
        if (pTriangle != nullptr) {
            *pTriangle = rHit.mParentTriangle;
        }

        if (pDst != nullptr) {
            *pDst = rHit.mHitPos;
        }

        return true;
    }
    bool isExistMapCollision(const TVec3f& rParam1, const TVec3f& rParam2) {
        return getCollisionDirector()->getCategoryKeeper(0)->checkStrikeLine(rParam1, rParam2, 1, nullptr, nullptr) != 0;
    }
    bool getFirstPolyOnLineBFast(const TVec3f& rStart, const TVec3f& rDirection, TVec3f* pPos, Triangle* pTriangle) {
        Triangle triangle;
        f32 remaining = rDirection.length();
        TVec3f direction(rDirection);
        direction.normalize();
        TVec3f start(rStart);
        TVec3f step(direction * 5000.0f);
        do {
            f32 length = 5000.0f;
            if (remaining < length) {
                length = remaining;
                step = direction * remaining;
            }

            u32 count = getNearPolyOnLineSort(start, start, step, nullptr);
            for (u32 i = 0; i < count; i++) {
                Triangle hitTriangle;
                TVec3f hitPos;
                if (getSortedPoly(&hitPos, &hitTriangle, i)) {
                    if (isWaterPolygon(&hitTriangle)) {
                        continue;
                    }

                    TVec3f backPos(hitPos - *getNormal(&hitTriangle) * 5.0f);
                    TVec3f normal(*getNormal(&hitTriangle) * 35.0f);
                    if (isExistMapCollision(backPos, normal)) {
                        continue;
                    }
                }

                if (pPos) {
                    *pPos = hitPos;
                }

                if (pTriangle) {
                    *pTriangle = hitTriangle;
                }

                return true;
            }

            remaining -= length;
            start += step;
        } while (!isNearZero(remaining));

        return false;
    }
    bool isWallCodeRebound(const Triangle* pTriangle) {
        return getWallCodeIndex(pTriangle) == CollisionWallCode_Rebound;
    }
    bool isBindedGroundSand(const LiveActor* pActor) {
        if (pActor->mBinder == nullptr) {
            return false;
        }

        if (!pActor->mBinder->isBindedGround()) {
            return false;
        }

        return isGroundCodeSand(&pActor->mBinder->mGroundInfo.mParentTriangle);
    }
    bool isGroundCodeSand(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == CollisionFloorCode_Sand;
    }
    const char* getFloorCodeString(const Triangle* pTriangle) {
        return getCollisionDirector()->mCode->getFloorCodeString(*pTriangle);
    }
    const char* getWallCodeString(const Triangle* pTriangle) {
        return getCollisionDirector()->mCode->getWallCodeString(*pTriangle);
    }
    const char* getSoundCodeString(const Triangle* pTriangle) {
        return getCollisionDirector()->mCode->getSoundCodeString(*pTriangle);
    }
    s32 getSoundCodeIndex(const JMapInfoIter& rIter) {
        return getCollisionDirector()->mCode->getSoundCode(rIter);
    }
    s32 getWallCodeIndex(const Triangle* pTriangle) {
        return getCollisionDirector()->mCode->getWallCode(pTriangle->getAttributes());
    }
    s32 getSoundCodeIndex(const Triangle* pTriangle) {
        return getCollisionDirector()->mCode->getSoundCode(pTriangle->getAttributes());
    }
    bool isWallCodeNoAction(const Triangle* pTriangle) {
        return getWallCodeIndex(pTriangle) == CollisionWallCode_NoAction;
    }
    bool isCodeSand(const Triangle* pTriangle) {
        return ((getSoundCodeIndex(pTriangle) == CollisionSoundCode_Sand || getSoundCodeIndex(pTriangle) == CollisionSoundCode_Beach) ||
                isGroundCodeSand(pTriangle)) ||
               isGroundCodeNoStampSand(pTriangle);
    }
    bool isBindedGroundDamageFire(const LiveActor* pActor) {
        if (pActor->mBinder == nullptr) {
            return false;
        }

        if (!pActor->mBinder->isBindedGround()) {
            return false;
        }

        return isGroundCodeDamageFire(&pActor->mBinder->mGroundInfo.mParentTriangle);
    }
    bool checkStrikePointToMap(const TVec3f& rParam1, HitInfo* pParam2) {
        return getCollisionDirector()->getCategoryKeeper(0)->checkStrikePoint(rParam1, pParam2) != 0;
    }
    bool calcMapGround(const TVec3f& rPos, TVec3f* pGround, f32 length) {
        TVec3f direction(0.0f, -length, 0.0f);
        return ::getFirstPolyOnLineCategory(pGround, nullptr, rPos, direction, nullptr, nullptr, 0);
    }
    bool isBindedDamageFire(const LiveActor* pActor) {
        if (pActor->mBinder == nullptr) {
            return false;
        }

        if (pActor->mBinder->isBindedGround() && isGroundCodeDamageFire(&pActor->mBinder->mGroundInfo.mParentTriangle)) {
            return true;
        }

        if (pActor->mBinder->isBindedWall() && isGroundCodeDamageFire(&pActor->mBinder->mWallInfo.mParentTriangle)) {
            return true;
        }

        if (pActor->mBinder->isBindedRoof() && isGroundCodeDamageFire(&pActor->mBinder->mRoofInfo.mParentTriangle)) {
            return true;
        }

        return false;
    }
    u32 getCameraID(const Triangle* pTriangle) {
        return getCollisionDirector()->mCode->getCameraID(*pTriangle);
    }
    bool isBindedGroundWater(const LiveActor* pActor) {
        if (pActor->mBinder == nullptr) {
            return false;
        }

        if (!pActor->mBinder->isBindedGround()) {
            return false;
        }

        return isGroundCodeWaterIter(pActor->mBinder->mGroundInfo.mParentTriangle.getAttributes());
    }
    bool isBindedGroundAreaMove(const LiveActor* pActor) {
        if (pActor->mBinder == nullptr) {
            return false;
        }

        if (!pActor->mBinder->isBindedGround()) {
            return false;
        }

        return isGroundCodeAreaMove(&pActor->mBinder->mGroundInfo.mParentTriangle);
    }
    bool isBindedGroundRailMove(const LiveActor* pActor) {
        if (pActor->mBinder == nullptr) {
            return false;
        }

        if (!pActor->mBinder->isBindedGround()) {
            return false;
        }

        return isGroundCodeRailMove(&pActor->mBinder->mGroundInfo.mParentTriangle);
    }
    bool trySetMoveLimitCollision(LiveActor* pActor) {
        TVec3f start(pActor->mPosition);
        TVec3f direction(pActor->mGravity);
        start -= direction * 150.0f;
        direction *= 1000.0f;
        if (getCollisionDirector()->getCategoryKeeper(3)->checkStrikeLine(start, direction, 0, nullptr, nullptr)) {
            pActor->mBinder->setExCollisionParts(getCollisionDirector()->getCategoryKeeper(3)->getStrikeInfo(0)->mParentTriangle.mParts);
            return true;
        }

        if (getCollisionDirector()->getCategoryKeeper(0)->checkStrikeLine(start, direction, 0, nullptr, nullptr)) {
            const HitInfo* pHit = getCollisionDirector()->getCategoryKeeper(0)->getStrikeInfo(0);
            CollisionParts* pSameParts = nullptr;
            CollisionParts* pParts = pHit->mParentTriangle.mParts;
            getCollisionDirector()->getCategoryKeeper(3)->searchSameHostParts(&pSameParts, pParts);
            pActor->mBinder->setExCollisionParts(pSameParts);
            return true;
        }

        return false;
    }
    bool isFallNextMove(const LiveActor* pActor, f32 param2, f32 param3, f32 param4, const TriangleFilterBase* pParam5) {
        return isFallNextMove(pActor->mPosition, pActor->mVelocity, pActor->mGravity, param2, param3, param4, pParam5);
    }
    bool isFallNextMove(const TVec3f& rPos, const TVec3f& rVelocity, const TVec3f& rGravity, f32 forwardDistance, f32 upperDistance,
                        f32 lowerDistance, const TriangleFilterBase* pTriangleFilter) {
        if (isNearZero(rGravity)) {
            return false;
        }

        TVec3f forward;
        forward.scaleAdd(-rGravity.dot(rVelocity), rGravity, rVelocity);
        if (isNearZero(forward)) {
            return false;
        }

        normalize(&forward);
        forward *= forwardDistance;
        TVec3f start;
        start = forward + rPos - rGravity * upperDistance;
        bool hit = getCollisionDirector()->getCategoryKeeper(0)->checkStrikeLine(start, rGravity * (upperDistance + lowerDistance), 1, nullptr,
                                                                                 pTriangleFilter) != 0;
        return !hit;
    }
    void calcVelocityMovingPoint(const Triangle* pTriangle, const TVec3f& rPoint, TVec3f* pVelocity) {
        if (isSameMtx(pTriangle->getBaseMtx()->toMtxPtr(), pTriangle->getPrevBaseMtx()->toMtxPtr())) {
            pVelocity->zero();
            return;
        }

        TVec3f localPoint;
        PSMTXMultVec(pTriangle->getBaseInvMtx()->toMtxPtr(), rPoint, &localPoint);
        TVec3f previousPoint;
        PSMTXMultVec(pTriangle->getPrevBaseMtx()->toMtxPtr(), &localPoint, &previousPoint);
        *pVelocity = rPoint - previousPoint;
    }
}
namespace Collision {
    s32 checkStrikeLineToSunshade(const TVec3f& rStart, const TVec3f& rDirection, s32 count, const CollisionPartsFilterBase* pPartsFilter,
                                  const TriangleFilterBase* pTriangleFilter) {
        return MR::getCollisionDirector()->getCategoryKeeper(1)->checkStrikeLine(rStart, rDirection, count, pPartsFilter, pTriangleFilter);
    }
}

