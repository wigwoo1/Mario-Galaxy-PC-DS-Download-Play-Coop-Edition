#pragma once
#include "Game/LiveActor/HitSensor.hpp"
// Original ActorSensorUtil messaging subset.
enum {
    ACTMES_PLAYER_ATTACK_START = 0x0,
    ACTMES_PLAYER_ATTACK_END = 0x1A,
    ACTMES_TAKE = 0x1E,
    ACTMES_TAKEN = 0x1F,
    ACTMES_APART = 0x21,
    ACTMES_THROW = 0x22,
    ACTMES_ENEMY_ATTACK_START = 0x4B,
    ACTMES_ENEMY_ATTACK_END = 0x65,
    ACTMES_PLAYER_PUNCH = 0x1,
    ACTMES_PLAYER_TRAMPLE = 0x2,
    ACTMES_PUSH = 0x29,
    ACTMES_ENEMY_ATTACK_FLIP = 0x50,
    ACTMES_ENEMY_ATTACK = 0x53,
    ACTMES_ITEM_GET = 0x87,
};
namespace MR {
void setSensorPos(HitSensor* pSensor, const TVec3f& rPos);
bool tryGetItem(HitSensor* pSender, HitSensor* pReceiver);
bool sendArbitraryMsg(u32 msg, HitSensor* pReceiver, HitSensor* pSender);
bool sendMsgPush(HitSensor* pReceiver, HitSensor* pSender);
bool sendMsgPlayerPunch(HitSensor* pReceiver, HitSensor* pSender);
bool sendMsgPlayerTrample(HitSensor* pReceiver, HitSensor* pSender);
bool sendMsgEnemyAttackMsgToDir(u32 msg, HitSensor* pReceiver, HitSensor* pSender, const TVec3f& rDir);
bool sendMsgEnemyAttackFlipToDir(HitSensor* pReceiver, HitSensor* pSender, const TVec3f& rDir);
bool sendMsgEnemyAttack(HitSensor* pReceiver, HitSensor* pSender);
}

