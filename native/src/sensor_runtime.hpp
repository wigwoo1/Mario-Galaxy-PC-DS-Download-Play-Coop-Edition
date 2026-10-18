#pragma once
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/SensorHitChecker.hpp"
// Native routing hook; no original collision-category routing/checker yet.
inline SensorGroup* currentNativeSensorGroup=nullptr;
namespace MR {void initHitSensorGroup(HitSensor* sensor){assetCheck(currentNativeSensorGroup!=nullptr,"No native sensor registration group");sensor->mSensorGroup=currentNativeSensorGroup;}}
class NativeSensorGroupScope {
    SensorGroup* previous;
public:
    explicit NativeSensorGroupScope(SensorGroup& group):previous(currentNativeSensorGroup){currentNativeSensorGroup=&group;}
    NativeSensorGroupScope(const NativeSensorGroupScope&)=delete;
    ~NativeSensorGroupScope(){currentNativeSensorGroup=previous;}
};
inline void sensorRuntimeTests(){
    struct Host:LiveActor {size_t messages=0;HitSensor* sender=nullptr;HitSensor* receiver=nullptr;bool receiveMessage(u32 message,HitSensor* from,HitSensor* to)override{++messages;sender=from;receiver=to;return message==42;}};
    Host host;SensorGroup group(8,"native test");{
        NativeSensorGroupScope scope(group);HitSensor first(1,2,10,&host),second(2,1,5,&host);assetCheck(!first.isValid()&&first.mSensorGroup==&group,"Original sensor initial state failed");
        first.validateBySystem();second.validateBySystem();assetCheck(first.isValid()&&group.mSensorCount==2,"Original sensor group registration failed");
        first.addHitSensor(&second);first.addHitSensor(&second);first.addHitSensor(&second);assetCheck(first.mSensorCount==2,"Original sensor contact capacity failed");
        assetCheck(first.receiveMessage(42,&second)&&host.sender==&second&&host.receiver==&first&&host.messages==1,"Original sensor message delivery failed");
        first.invalidate();assetCheck(!first.isValid()&&group.mSensorCount==1&&first.mSensorCount==0,"Original sensor host invalidation failed");first.validate();first.setType(9);assetCheck(first.isType(9)&&group.mSensorCount==2,"Original sensor type re-registration failed");
        first.addHitSensor(&second);group.clear();assetCheck(first.mSensorCount==0,"Original sensor group contact clear failed");first.invalidateBySystem();second.invalidateBySystem();assetCheck(group.mSensorCount==0,"Original sensor system removal failed");delete[] first.mSensors;delete[] second.mSensors;
    }delete[] group.mSensors;assetCheck(currentNativeSensorGroup==nullptr,"Native sensor registration scope not restored");
}
