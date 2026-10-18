#include "Game/Map/HitInfo.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Map/KCollision.hpp"
#include "Game/Util/MathUtil.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

JMapInfoIter Triangle::getAttributes() const {
    if (mIdx == -1) {
        return JMapInfoIter();
    }

    return mParts->mServer->getAttributes(mIdx);
}
const char* Triangle::getHostName() const {
    return mParts->getHostName();
}
s32 Triangle::getHostPlacementZoneID() const {
    return mParts->getPlacementZoneID();
}
bool Triangle::isValid() const {
    return mIdx != 0xFFFFFFFF;
}
