#include "Game/LiveActor/SensorHitChecker.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
// Original checker constructor/destructor, movement and overlap-method subset.
constexpr u32 ATYPE_EYE=0x7F;
// NativeScene schedules this checker directly; original scene-category connection is pending.
void SensorHitChecker::init(const JMapInfoIter&) {}
namespace MR {inline bool isClipped(const LiveActor* host){return host->isClipped();}}
void SensorHitChecker::movement() {
    mPlayerGroup->clear();
    mRideGroup->clear();
    mEyeGroup->clear();
    mSimpleGroup->clear();
    mMapObjGroup->clear();
    mCharacterGroup->clear();
    doObjColGroup(mPlayerGroup, mCharacterGroup);
    doObjColGroup(mPlayerGroup, mMapObjGroup);
    doObjColGroup(mPlayerGroup, mRideGroup);
    doObjColGroup(mPlayerGroup, mSimpleGroup);
    doObjColGroup(mPlayerGroup, mEyeGroup);
    doObjColGroup(mRideGroup, mCharacterGroup);
    doObjColGroup(mRideGroup, mMapObjGroup);
    doObjColGroup(mRideGroup, mSimpleGroup);
    doObjColGroup(mRideGroup, mEyeGroup);
    doObjColGroup(mEyeGroup, mCharacterGroup);
    doObjColGroup(mEyeGroup, mMapObjGroup);
    doObjColGroup(mEyeGroup, mSimpleGroup);
    doObjColGroup(mCharacterGroup, mMapObjGroup);
    doObjColInSameGroup(mCharacterGroup);
}

void SensorHitChecker::doObjColGroup(SensorGroup* pGroup1, SensorGroup* pGroup2) const {
    HitSensor* curGroup1Sensor;
    HitSensor* curGroup2Sensor;
    s32 group1SensorCount = pGroup1->mSensorCount;
    for (s32 i = 0; i < group1SensorCount; i++) {
        curGroup1Sensor = pGroup1->mSensors[i];
        bool group1Sensorvalidated = curGroup1Sensor->mValidByHost && curGroup1Sensor->mValidBySystem;

        if (group1Sensorvalidated && !MR::isClipped(curGroup1Sensor->mHost)) {
            s32 group2SensorCount = pGroup2->mSensorCount;
            for (s32 x = 0; x < group2SensorCount; x++) {
                curGroup2Sensor = pGroup2->mSensors[x];
                bool group2Validated = curGroup2Sensor->mValidByHost && curGroup2Sensor->mValidBySystem;

                if (group2Validated && !MR::isClipped(curGroup2Sensor->mHost)) {
                    checkAttack(curGroup1Sensor, curGroup2Sensor);
                }
            }
        }
    }
}

void SensorHitChecker::doObjColInSameGroup(SensorGroup* pSensorGroup) const {
    HitSensor* pFirstSensor;
    HitSensor* pSecondSensor;
    s32 sensorGroupCount = pSensorGroup->mSensorCount;
    for (s32 i = 0; i < sensorGroupCount; i++) {
        pFirstSensor = pSensorGroup->mSensors[i];
        bool isFirstSensorValid = pFirstSensor->mValidByHost && pFirstSensor->mValidBySystem;

        if (isFirstSensorValid && !MR::isClipped(pFirstSensor->mHost)) {
            for (s32 x = i; x < sensorGroupCount; x++) {
                pSecondSensor = pSensorGroup->mSensors[x];
                bool isSecondSensorValid = pSecondSensor->mValidByHost && pSecondSensor->mValidBySystem;

                if (isSecondSensorValid && !MR::isClipped(pSecondSensor->mHost)) {
                    checkAttack(pFirstSensor, pSecondSensor);
                }
            }
        }
    }
}

void SensorHitChecker::checkAttack(HitSensor* pSensor1, HitSensor* pSensor2) const {
    if (pSensor1->mHost != pSensor2->mHost) {
        f32 xPos = pSensor1->mPosition.x - pSensor2->mPosition.x;
        f32 yPos = pSensor1->mPosition.y - pSensor2->mPosition.y;
        f32 zPos = pSensor1->mPosition.z - pSensor2->mPosition.z;
        f32 totalSize = pSensor1->getRadius() + pSensor2->getRadius();

        if (!((((xPos * xPos) + (yPos * yPos)) + (zPos * zPos)) >= (totalSize * totalSize))) {
            if (!pSensor2->isType(ATYPE_EYE)) {
                pSensor1->addHitSensor(pSensor2);
            }

            if (!pSensor1->isType(ATYPE_EYE)) {
                pSensor2->addHitSensor(pSensor1);
            }
        }
    }
}

SensorHitChecker::~SensorHitChecker() {
}

SensorHitChecker::SensorHitChecker(const char* pName) : NameObj(pName) {
    mPlayerGroup = nullptr;
    mRideGroup = nullptr;
    mEyeGroup = nullptr;
    mSimpleGroup = nullptr;
    mMapObjGroup = nullptr;
    mCharacterGroup = nullptr;

    mPlayerGroup = new SensorGroup(0x10, "Player");
    mRideGroup = new SensorGroup(0x80, "Ride");
    mEyeGroup = new SensorGroup(0x200, "Eye");
    mSimpleGroup = new SensorGroup(0x800, "Simple");
    mMapObjGroup = new SensorGroup(0x400, "MapObj");
    mCharacterGroup = new SensorGroup(0x400, "Character");
}

