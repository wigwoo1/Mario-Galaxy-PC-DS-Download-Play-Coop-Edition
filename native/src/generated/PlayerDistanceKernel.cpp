#include <Game/Util/ActorMovementUtil.hpp>
#include <Game/LiveActor/LiveActor.hpp>
#include <Game/LiveActor/HitSensor.hpp>
#include <Game/Scene/SceneObjHolder.hpp>
#include <Game/Util/PlayerUtil.hpp>
#include <Game/Util/ObjUtil.hpp>
#include <Game/Util/MathUtil.hpp>
namespace MR {    bool isNearPlayer(const TVec3f& rPos, f32 distance) {
        if (MR::isPlayerHidden()) {
            return false;
        }

        f32 squaredDistance = MR::getPlayerPos()->squared(rPos);

        return squaredDistance < distance * distance;
    }
    bool isNearPlayer(const LiveActor* pActor, f32 distance) {
        if (MR::isPlayerHidden()) {
            return false;
        }

        return MR::isNearPlayerAnyTime(pActor, distance);
    }
    f32 calcDistanceToPlayer(const LiveActor* pActor) {
        if (!MR::isExistSceneObj(SceneObj_MarioHolder)) {
            return FLOAT_MAX;
        }

        return pActor->mPosition.distance(*MR::getPlayerPos());
    }
    bool isNear(const HitSensor* pSensorA, const HitSensor* pSensorB, f32 distance) {
        f32 squaredDistance = pSensorA->mPosition.squared(pSensorB->mPosition);

        return squaredDistance < distance * distance;
    }
    bool isNear(const LiveActor* pActorA, const LiveActor* pActorB, f32 distance) {
        f32 squaredDistance = pActorA->mPosition.squared(pActorB->mPosition);

        return squaredDistance < distance * distance;
    }
    bool isNear(const LiveActor* pActor, const TVec3f& rTrans, f32 distance) {
        f32 squaredDistance = pActor->mPosition.squared(rTrans);

        return squaredDistance < distance * distance;
    }
    bool isNearPlayerAnyTime(const LiveActor* pActor, f32 distance) {
        return isNear(pActor, *MR::getPlayerPos(), distance);
    }
}
