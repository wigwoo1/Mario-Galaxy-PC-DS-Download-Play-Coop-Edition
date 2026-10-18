#include <Game/LiveActor/HitSensor.hpp>
#include <Game/Util/ActorSensorUtil.hpp>
#include <Game/Scene/SceneObjHolder.hpp>
namespace MR {bool isSensorPlayer(const HitSensor* pSensor) {
        return pSensor->isType(ATYPE_PLAYER);
    }bool isSensorRide(const HitSensor* pSensor) {
        if (pSensor->mType > ATYPE_RIDE_START && pSensor->mType < ATYPE_RIDE_END) {
            return true;
        }

        return false;
    }bool isSensorMapObj(const HitSensor* pSensor) {
        if (pSensor->mType > ATYPE_MAPOBJ_START && pSensor->mType < ATYPE_MAPOBJ_END) {
            return true;
        }

        return false;
    }bool isSensorAutoRush(const HitSensor* pSensor) {
        if (pSensor->mType > ATYPE_AUTO_RUSH_OBJ_START && pSensor->mType < ATYPE_AUTO_RUSH_OBJ_END) {
            return true;
        }

        return false;
    }bool isSensorRush(const HitSensor* pSensor) {
        if (pSensor->mType > ATYPE_RUSH_OBJ_START && pSensor->mType < ATYPE_RUSH_OBJ_END) {
            return true;
        }

        return false;
    }}
#include <Game/NameObj/NameObj.hpp>
SceneObjHolder::SceneObjHolder() {
    for (int i = 0; i < SceneObj_NumMax; i++) {
        mObj[i] = nullptr;
    }
}NameObj* SceneObjHolder::getObj(int id) const {
    return mObj[id];
}NameObj* SceneObjHolder::create(int id) {
    NameObj* pObj = mObj[id];

    if (pObj != nullptr) {
        return pObj;
    }

    pObj = newEachObj(id);
    pObj->initWithoutIter();

    mObj[id] = pObj;

    return pObj;
}void NameObj::initWithoutIter() {
    init(JMapInfoIter());
}
