#include "sensor_messages.hpp"
// Function bodies extracted unchanged from Petari ActorSensorUtil.cpp.
namespace MR {
    void setSensorPos(HitSensor* pSensor, const TVec3f& rPos) {
        pSensor->mPosition.set(rPos);
    }
    bool tryGetItem(HitSensor* pSender, HitSensor* pReceiver) {
        return pReceiver->receiveMessage(ACTMES_ITEM_GET, pSender);
    }
    bool sendArbitraryMsg(u32 msg, HitSensor* pReceiver, HitSensor* pSender) {
        return pReceiver->receiveMessage(msg, pSender);
    }
    bool sendMsgPush(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(ACTMES_PUSH, pReceiver, pSender);
    }
    bool sendMsgPlayerPunch(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(ACTMES_PLAYER_PUNCH, pReceiver, pSender);
    }
    bool sendMsgPlayerTrample(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(ACTMES_PLAYER_TRAMPLE, pReceiver, pSender);
    }
    bool sendMsgEnemyAttackMsgToDir(u32 msg, HitSensor* pReceiver, HitSensor* pSender, const TVec3f& rDir) {
        TVec3f senderPos = pSender->mPosition;
        pSender->mPosition.sub(pReceiver->mPosition, rDir);

        bool isSent = pReceiver->receiveMessage(msg, pSender);
        setSensorPos(pSender, senderPos);

        return isSent;
    }
    bool sendMsgEnemyAttackFlipToDir(HitSensor* pReceiver, HitSensor* pSender, const TVec3f& rDir) {
        return sendMsgEnemyAttackMsgToDir(ACTMES_ENEMY_ATTACK_FLIP, pReceiver, pSender, rDir);
    }
    bool sendMsgEnemyAttack(HitSensor* pReceiver, HitSensor* pSender) {
        return sendArbitraryMsg(ACTMES_ENEMY_ATTACK, pReceiver, pSender);
    }
}
