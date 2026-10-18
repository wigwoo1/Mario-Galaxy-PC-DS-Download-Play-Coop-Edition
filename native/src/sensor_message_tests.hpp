#pragma once
#include "sensor_messages.hpp"
inline void sensorMessageTests(){
 struct Host:LiveActor {bool accepted=true;u32 message=0;HitSensor* from=nullptr;HitSensor* to=nullptr;TVec3f seen;int messages=0;bool receiveMessage(u32 m,HitSensor* sender,HitSensor* receiver)override{++messages;message=m;from=sender;to=receiver;seen=sender->mPosition;return accepted;}void attackSensor(HitSensor* sender,HitSensor* receiver)override{MR::sendMsgPush(receiver,sender);}};
 Host a,b;NativeSensorWorld world;auto& ak=world.createKeeperForType(1,1);auto& bk=world.createKeeperForType(0x46,1);auto& sender=ak.add("body",1,2,1,a,{});auto& receiver=bk.add("body",0x46,2,1,b,{});ak.validate();bk.validate();world.tick();assetCheck(b.message==ACTMES_PUSH&&b.from==&sender&&b.to==&receiver&&b.messages==1,"Contact-to-original-message pipeline failed");
 assetCheck(MR::sendMsgPlayerPunch(&receiver,&sender)&&b.message==ACTMES_PLAYER_PUNCH,"Original punch message failed");b.accepted=false;assetCheck(!MR::sendMsgPlayerTrample(&receiver,&sender)&&b.message==ACTMES_PLAYER_TRAMPLE,"Original rejected trample reply failed");assetCheck(!MR::tryGetItem(&sender,&receiver)&&b.message==ACTMES_ITEM_GET,"Original item message argument order failed");
 sender.mPosition={10,20,30};receiver.mPosition={100,200,300};assetCheck(!MR::sendMsgEnemyAttackFlipToDir(&receiver,&sender,{2,3,4})&&b.seen.x==98&&b.seen.y==197&&b.seen.z==296&&sender.mPosition.x==10&&sender.mPosition.y==20&&sender.mPosition.z==30,"Original directional attack temporary position or restoration failed");b.accepted=true;assetCheck(MR::sendMsgEnemyAttack(&receiver,&sender)&&b.message==ACTMES_ENEMY_ATTACK,"Original enemy attack reply failed");
}

