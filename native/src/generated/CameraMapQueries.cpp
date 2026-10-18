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


namespace MR {
    s32 getCameraCodeIndex(const Triangle* pTriangle) {
        return getCollisionDirector()->mCode->getCameraCode(pTriangle->getAttributes());
    }
    bool isCameraCodeThrough(const Triangle* pTriangle) {
        return getCameraCodeIndex(pTriangle) == CollisionCameraCode_Through;
    }
}
namespace Collision {
    s32 checkStrikeLineToMap(const TVec3f& rStart, const TVec3f& rDirection, s32 count, const CollisionPartsFilterBase* pPartsFilter,
                             const TriangleFilterBase* pTriangleFilter) {
        return MR::getCollisionDirector()->getCategoryKeeper(0)->checkStrikeLine(rStart, rDirection, count, pPartsFilter, pTriangleFilter);
    }
}

namespace MR {
    s32 getFloorCodeIndex(const Triangle* pTriangle) {
        return getCollisionDirector()->mCode->getFloorCode(pTriangle->getAttributes());
    }
    s32 getFloorCodeIndex(const JMapInfoIter& rIter) {
        return getCollisionDirector()->mCode->getFloorCode(rIter);
    }
    bool isGroundCodeDeath(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == CollisionFloorCode_Death;
    }
    bool isGroundCodeDamage(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == CollisionFloorCode_DamageNormal;
    }
    bool isGroundCodeDamageFire(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == CollisionFloorCode_DamageFire;
    }
    bool isGroundCodeFireDance(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == CollisionFloorCode_FireDance;
    }
    bool isGroundCodeDamageElectric(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == CollisionFloorCode_DamageElectric;
    }
    bool isGroundCodeSinkDeath(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == CollisionFloorCode_SinkDeath;
    }
    bool isGroundCodeSinkDeathMud(const Triangle* pTriangle) {
        return getFloorCodeIndex(pTriangle) == CollisionFloorCode_SinkDeathMud;
    }
}

