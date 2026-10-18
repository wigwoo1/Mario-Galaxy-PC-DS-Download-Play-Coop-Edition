#include "Game/LiveActor/SensorHitChecker.hpp"
#include "Game/LiveActor/HitSensor.hpp"
// Extracted original classifier and its predicate/enum dependency subset.
enum {
    ATYPE_PLAYER = 0x1,
    ATYPE_RIDE_START = 0x8,
    ATYPE_RIDE_END = 0x12,
    ATYPE_ENEMY_SIMPLE = 0x15,
    ATYPE_PLAYER_AUTO_JUMP = 0x1F,
    ATYPE_MAPOBJ_START = 0x45,
    ATYPE_MAP_OBJ_SIMPLE = 0x47,
    ATYPE_COIN = 0x4A,
    ATYPE_STAR_PIECE = 0x4C,
    ATYPE_MAPOBJ_END = 0x5F,
    ATYPE_AUTO_RUSH_OBJ_START = 0x60,
    ATYPE_AUTO_RUSH_OBJ_END = 0x6E,
    ATYPE_RUSH_OBJ_START = 0x6F,
    ATYPE_RUSH_OBJ_END = 0x74,
    ATYPE_EYE = 0x7F,
};
namespace MR {
    bool isSensorPlayer(const HitSensor* pSensor) {
        return pSensor->isType(ATYPE_PLAYER);
    }

    bool isSensorRide(const HitSensor* pSensor) {
        if (pSensor->mType > ATYPE_RIDE_START && pSensor->mType < ATYPE_RIDE_END) {
            return true;
        }

        return false;
    }

    bool isSensorMapObj(const HitSensor* pSensor) {
        if (pSensor->mType > ATYPE_MAPOBJ_START && pSensor->mType < ATYPE_MAPOBJ_END) {
            return true;
        }

        return false;
    }

    bool isSensorRush(const HitSensor* pSensor) {
        if (pSensor->mType > ATYPE_RUSH_OBJ_START && pSensor->mType < ATYPE_RUSH_OBJ_END) {
            return true;
        }

        return false;
    }

    bool isSensorAutoRush(const HitSensor* pSensor) {
        if (pSensor->mType > ATYPE_AUTO_RUSH_OBJ_START && pSensor->mType < ATYPE_AUTO_RUSH_OBJ_END) {
            return true;
        }

        return false;
    }

}
void SensorHitChecker::initGroup(HitSensor* pSensor) {
    if (MR::isSensorPlayer(pSensor)) {
        pSensor->mSensorGroup = mPlayerGroup;
    } else if (MR::isSensorRide(pSensor)) {
        pSensor->mSensorGroup = mRideGroup;
    } else if (pSensor->isType(ATYPE_EYE)) {
        pSensor->mSensorGroup = mEyeGroup;
    } else if (pSensor->isType(ATYPE_COIN) || pSensor->isType(ATYPE_STAR_PIECE) || pSensor->isType(ATYPE_ENEMY_SIMPLE) ||
               pSensor->isType(ATYPE_MAP_OBJ_SIMPLE) || pSensor->isType(ATYPE_PLAYER_AUTO_JUMP) || MR::isSensorRush(pSensor) ||
               MR::isSensorAutoRush(pSensor)) {
        pSensor->mSensorGroup = mSimpleGroup;
    } else {
        if (MR::isSensorMapObj(pSensor)) {
            pSensor->mSensorGroup = mMapObjGroup;
        } else {
            pSensor->mSensorGroup = mCharacterGroup;
        }
    }
}

