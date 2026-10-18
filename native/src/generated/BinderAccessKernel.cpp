#include <Game/LiveActor/Binder.hpp>
#include <Game/Map/HitInfo.hpp>
#include <algorithm>
#include <functional>
#include <stdexcept>
void Binder::setTriangleFilter(TriangleFilterBase* pFilter) {
    mTriangleFilter = pFilter;
}
void Binder::setCollisionPartsFilter(CollisionPartsFilterBase* pFilter) {
    mCollisionPartsFilter = pFilter;
}
const HitInfo* Binder::getPlane(int index) const {
    return &mPlane[index];
}
u32 Binder::copyPlaneArrayAndSortingSensor(HitInfo** pPlanes, u32 capacity) { const u32 required=_24==0?u32(isBindedGround())+u32(isBindedWall())+u32(isBindedRoof()):mPlaneNum; if(required>capacity)throw std::length_error("Binder plane output capacity"); if(!required)return 0; if(!pPlanes)throw std::invalid_argument("Null Binder plane output"); 
    if (_24 == 0) {
        u32 count = 0;
        if (isBindedGround()) {
            pPlanes[count++] = &mGroundInfo;
        }

        if (isBindedWall()) {
            pPlanes[count++] = &mWallInfo;
        }

        if (isBindedRoof()) {
            pPlanes[count++] = &mRoofInfo;
        }

        std::sort(pPlanes, pPlanes + count, compSensor);
        return count;
    }

    for (u32 i = 0; i < mPlaneNum; i++) {
        pPlanes[i] = &mPlane[i];
    }

    std::sort(pPlanes, pPlanes + mPlaneNum, compSensor);
    return mPlaneNum;
}
bool Binder::compSensor(const HitInfo* pPlane1, const HitInfo* pPlane2) {
    return std::less<HitSensor*>{}(pPlane2->mParentTriangle.mSensor,pPlane1->mParentTriangle.mSensor);
}

