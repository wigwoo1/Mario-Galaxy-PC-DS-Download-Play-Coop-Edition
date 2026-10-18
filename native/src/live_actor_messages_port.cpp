#include "Game/LiveActor/LiveActor.hpp"
#include "sensor_messages.hpp"
// Original receiveMessage dispatcher; full LiveActor lifecycle remains unported.
bool LiveActor::receiveMessage(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == ACTMES_PUSH) {
        return receiveMsgPush(pSender, pReceiver);
    }

    bool isMsgPlayerAttack = msg > ACTMES_PLAYER_ATTACK_START && msg < ACTMES_PLAYER_ATTACK_END;

    if (isMsgPlayerAttack) {
        return receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }

    bool isMsgEnemyAttack = msg > ACTMES_ENEMY_ATTACK_START && msg < ACTMES_ENEMY_ATTACK_END;

    if (isMsgEnemyAttack) {
        return receiveMsgEnemyAttack(msg, pSender, pReceiver);
    }

    if (msg == ACTMES_TAKE) {
        return receiveMsgTake(pSender, pReceiver);
    }

    if (msg == ACTMES_TAKEN) {
        return receiveMsgTaken(pSender, pReceiver);
    }

    if (msg == ACTMES_THROW) {
        return receiveMsgThrow(pSender, pReceiver);
    }

    if (msg == ACTMES_APART) {
        return receiveMsgApart(pSender, pReceiver);
    }

    return receiveOtherMsg(msg, pSender, pReceiver);
}

