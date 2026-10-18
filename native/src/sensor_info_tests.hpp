#pragma once
#include "Game/LiveActor/HitSensorInfo.hpp"
inline void sensorInfoTests(){
 struct Host:LiveActor {Mtx matrix={{0,-1,0,50},{1,0,0,60},{0,0,1,70}};bool matrixEnabled=false,dead=false;int callbacks=0,attacks=0;bool receiveMessage(u32,HitSensor*,HitSensor*)override{return false;}bool isDead()const override{return dead;}MtxPtr getBaseMtx()const override{return matrixEnabled?const_cast<MtxPtr>(matrix):nullptr;}void updateHitSensor(HitSensor* s)override{++callbacks;s->mPosition=TVec3f(9,8,7);}void attackSensor(HitSensor*,HitSensor*)override{++attacks;}};
 Host host,target;host.mPosition=TVec3f(10,20,30);SensorGroup group(4,"positions");{
 NativeSensorGroupScope scope(group);HitSensor sensor(1,2,1,&host),other(1,1,1,&target);HitSensorInfo info("body",&sensor,nullptr,nullptr,TVec3f(2,3,4),false);
 auto check=[&](float x,float y,float z){assetCheck(sensor.mPosition.x==x&&sensor.mPosition.y==y&&sensor.mPosition.z==z,"Sensor position transform failed");};
 info.update();check(12,23,34);host.matrixEnabled=true;info.update();check(7,22,34);
 TVec3f external(100,200,300);HitSensorInfo positioned("external",&sensor,&external,nullptr,TVec3f(2,3,4),false);positioned.update();check(97,202,304);external.x=110;positioned.update();check(107,202,304);
 HitSensorInfo matrix("matrix",&sensor,nullptr,host.matrix,TVec3f(2,3,4),false);matrix.update();check(47,62,74);
 HitSensorInfo callback("callback",&sensor,nullptr,nullptr,TVec3f(),true);callback.update();check(9,8,7);assetCheck(host.callbacks==1,"Sensor callback not dispatched");
 sensor.addHitSensor(&other);info.doObjCol();assetCheck(host.attacks==1,"Sensor contact attack not dispatched");target.dead=true;info.doObjCol();assetCheck(host.attacks==1,"Dead contact attack was dispatched");assetCheck(info.mHashCode==3029410,"Sensor name hash failed");delete[] sensor.mSensors;delete[] other.mSensors;
 }delete[] group.mSensors;
}
