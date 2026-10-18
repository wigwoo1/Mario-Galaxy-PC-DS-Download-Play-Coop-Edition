#include "Game/Util/PlayerUtil.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioAccess.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Camera/CameraTargetArg.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Player/RushEndInfo.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
namespace MR {
    LiveActor* getPlayerDemoActor() {
        return MarioAccess::getPlayerActor();
    }
    bool isActorOnPlayer(const LiveActor* pActor) {
        return MarioAccess::isOnActor(pActor);
    }
    bool checkPlayerSwingTrigger() {
        return MarioAccess::getPlayerActor()->_1E1;
    }
void endBindAndPlayerForceWeakGravityJumpInputOff(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);
        info._20 |= 0xC0000000;

        MarioAccess::endRush(&info);
    }
    const TVec3f* getPlayerGroundNormal() {
        return getPlayerGroundingPolygon()->getFaceNormal();
    }
    void pushPlayer(const TVec3f& rAccel) {
        MarioAccess::addVelocity(rAccel);
    }
    void forceKillPlayerByGroundRace() {
        return MarioAccess::forceKill(5, 0);
    }
    void startLevelSoundPlayer(const char* pName, s32 param1) {
        MR::startLevelSound(MarioAccess::getPlayerActor(), pName, param1);
    }
    bool isPlayerTeresaDisappear() {
        return MarioAccess::isTeresaDisappear();
    }
    f32 getPlayerHitRadius() {
        return MarioAccess::getPlayerActor()->mPlayerMode == (s16)6 ? 100.0f : 60.0f;
    }
    void pushPlayerFromArea(const TVec3f& rAccel) {
        MarioAccess::addVelocityFromArea(rAccel);
    }
bool isPlayerJumpRising() {
        return MarioAccess::getPlayerActor()->isJumpRising();
    }
void setPlayerFrontTargetVec(const TVec3f& rTarget, s32 param2) {
        MarioAccess::setFrontVecTarget(rTarget, param2);
    }
void endBindAndPlayerWeakGravityJump(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);

        MarioAccess::endRush(&info);
    }
void getPlayerGroundPos(TVec3f* pPos) {
        MarioAccess::getPlayerActor()->getGroundPos(pPos);
    }
void tryPlayerPullActor(HitSensor* pSensor) {
        MarioAccess::getPlayerActor()->tryTornadoPull(pSensor);
    }
void getPlayerTakePos(TVec3f* pPos) {
        return MarioAccess::getTakePos(pPos);
    }
void getPlayerThrowVec(TVec3f* pVec) {
        MarioAccess::getThrowVec(pVec);
    }
void endBindAndPlayerJumpWithRollLanding(LiveActor* pActor, const TVec3f& rParam2, u32 param3) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, param3);
        info._20 |= 0x400000;

        MarioAccess::endRush(&info);
    }
void initPlayerAfterOpeningDemo() {
        MarioAccess::getPlayerActor()->initAfterOpeningDemo();
    }
bool isPlayerHidden() {
        return isPlayerElementMode(6);
    }
bool isPlayerOnPress() {
        return MarioAccess::isOnPress();
    }
void forceJumpPlayer(const TVec3f& rParam1) {
        MarioAccess::forceJump(rParam1, 0);
    }
bool isFpViewChangingFailure() {
        return MarioAccess::isFpViewChangingFailure();
    }
void resetPlayerStatus() {
        MarioAccess::getPlayerActor()->resetCondition();
    }
void forceKillPlayerByAbyss() {
        return MarioAccess::forceKill(0, 0);
    }
void startSoundPlayer(const char* pName, s32 param1) {
        MR::startSound(MarioAccess::getPlayerActor(), pName, param1);
    }
bool isBckStoppedPlayer() {
        return MR::isBckStopped(MarioAccess::getPlayerActor());
    }
void tryPlayerKillTakingActor() {
        MarioAccess::killTakingActor();
    }
bool isOnPlayer(const HitSensor* pSensor) {
        Triangle* polygon = getPlayerGroundingPolygon();

        if (polygon != nullptr) {
            return polygon->mSensor == pSensor;
        }

        return false;
    }
bool isOffPlayerControl() {
        return MarioAccess::isOffControl();
    }
bool isPlayerRefuseTalk() {
        return MarioAccess::getPlayerActor()->isRefuseTalk();
    }
bool isPlayerElementModeNormal() {
        return isPlayerElementMode(PlayerMode_Normal);
    }
bool isPlayerDead() {
        return !MarioAccess::getPlayerActor()->isEnableNerveChange();
    }
bool isPlayerDamaging() {
        return MarioAccess::getPlayerActor()->isDamaging();
    }
bool isPlayerInAreaObj(const char* pName) {
        return isInAreaObj(pName, MarioAccess::getPlayerActor()->mPosition);
    }
void incPlayerLife(u32 add) {
        MarioAccess::incLife(add);
    }
TVec3f* getPlayerShadowRotate() {
        return &MarioAccess::getPlayerActor()->_A18;
    }
TVec3f* getPlayerCenterPos() {
        return &MarioAccess::getPlayerActor()->_2A0;
    }
TVec3f* getPlayerRotate() {
        return &MarioAccess::getPlayerActor()->mRotation;
    }
TVec3f* getPlayerVelocity() {
        return MarioAccess::getVelocity();
    }
const TVec3f* getPlayerGravity() {
        return &MarioAccess::getPlayerActor()->getGravityVec();
    }
void getPlayerUpVec(TVec3f* pVec) {
        MarioAccess::getPlayerActor()->getUpVec(pVec);
    }
void getStarPiecePlayer() {
        MarioAccess::addStarPiece();
    }
void notifyDirectGetStarPiecePlayer() {
        MarioAccess::getStarPieceDirect();
    }
void changePlayerItemStatus(s32 param1) {
        MarioAccess::changeItemStatus(param1);
    }
void incPlayerOxygen(u32 add) {
        MarioAccess::incOxygen(add);
    }
void stopSoundPlayer(const char* pName, u32 param1) {
        MR::stopSound(MarioAccess::getPlayerActor(), pName, param1);
    }
void setPlayerBaseMtx(MtxPtr pMtx) {
        MarioAccess::setBaseMtx(pMtx);
    }
MtxPtr getPlayerBaseMtx() {
        return MarioAccess::getBaseMtx();
    }
void startBckPlayer(const char* pName, s32 param2) {
        MarioAccess::changeAnimationE(pName, param2);
    }
void startBckPlayer(const char* pName, const char* pParam2) {
        if (pParam2 != nullptr) {
            MarioAccess::changeAnimationE(pName, pParam2);
        } else {
            MarioAccess::changeAnimationE(pName, pName);
        }
    }
void setPlayerSwingPermission(bool permission) {
        MarioAccess::getPlayerActor()->_EEB = permission;
    }
bool isPlayerInBind() {
        return MarioAccess::isInRush();
    }
bool isPlayerElementModeBee() {
        return isPlayerElementMode(PlayerMode_Bee);
    }
void startBckPlayer(const char* pName, const BckCtrlData& rCtrlData) {
        MarioAccess::changeAnimationE(pName, rCtrlData);
    }
void startBckPlayerJ(const char* pName) {
        MarioAccess::changeAnimationJ(pName);
    }
Triangle* getPlayerGroundingPolygon() {
        return MarioAccess::getGroundingPolygon(0);
    }
u16 getPlayerMovementTimer() {
        return MarioAccess::getPlayerActor()->_378;
    }
TVec3f* getPlayerLastMove() {
        return MarioAccess::getLastMove();
    }
bool isPlayerInWaterMode() {
        return MarioAccess::isInWaterMode();
    }
bool isPlayerOnWaterSurface() {
        return MarioAccess::isOnWaterSurface();
    }
bool isPlayerFlying() {
        return MarioAccess::isFlying();
    }
void stopPlayerFpView() {
        return MarioAccess::stopFpView();
    }
void setCameraTargetToPlayer(CameraTargetArg* pTarget) {
        pTarget->mMarioActor = MarioAccess::getPlayerActor();
        pTarget->mTargetObj = nullptr;
        pTarget->mTargetMtx = nullptr;
        pTarget->mLiveActor = nullptr;
    }
CubeCameraArea* getCameraCube() {
        return MarioAccess::getCameraCubeCode();
    }
bool isExistMario() {
        return MR::isExistSceneObj(SceneObj_MarioHolder);
    }
bool isPlayerElementModeHopper() {
        return isPlayerElementMode(PlayerMode_Hopper);
    }
bool isPlayerElementModeTeresa() {
        return isPlayerElementMode(PlayerMode_Teresa);
    }
bool isPlayerSwimming() {
        return MarioAccess::isSwimming();
    }
bool isOnGroundPlayer() {
        return MarioAccess::isOnGround(0);
    }
void forceKillPlayerByWaterRace() {
        return MarioAccess::forceKill(1, 0);
    }
void calcPlayerJointMtx(TPos3f* pMtx, const char* pName) {
        pMtx->set(MarioAccess::getJointMtx(pName));
    }
bool isPlayerSwingAction() {
        return MarioAccess::isSwingAction();
    }
bool isPlayerParalyzing() {
        return MarioAccess::isParalyzing();
    }
bool isPlayerInRush() {
        return MarioAccess::isInRush();
    }
bool isPlayerElementMode(s32 mode) {
        return MarioAccess::getPlayerActor()->mPlayerMode == mode;
    }
bool isPlayerDisableFpView() {
        return MarioAccess::isDisableFpView();
    }
bool isPlayerNeedBrakingCamera() {
        return MarioAccess::isNeedBrakingCamera();
    }
bool isPlayerConfrontDeath() {
        return MarioAccess::isConfrontDeath();
    }
void endBindAndPlayerJump(LiveActor* pActor, const TVec3f& rParam2, u32 param3) {
        RushEndInfo info = RushEndInfo(pActor, 2, rParam2, true, param3);

        MarioAccess::endRush(&info);
    }
void endBindAndSpinDriverJump(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);
        info._20 |= 0xC0000000;

        MarioAccess::endRush(&info);
    }
void endBindAndPlayerWeakGravityLimitJump(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);
        info._20 |= 0x800000;

        MarioAccess::endRush(&info);
    }
bool isPlayerElementModeInvincible() {
        return isPlayerElementMode(PlayerMode_Invincible);
    }
void setPlayerStateWait() {
        MarioAccess::setStateWait();
    }
void setPlayerFrontVec(const TVec3f& rVec, s32 param2) {
        MarioAccess::setFrontVecKeepUp(rVec, param2);
    }
void setPlayerSpot(f32 param1, u32 param2) {
        MarioAccess::setSpot(param1, param2);
    }
void setPlayerPos(const char* pParam1) {
        TPos3f mtx;
        mtx.identity();

        MR::findNamePos(pParam1, mtx.toMtxPtr());
        MarioAccess::setBaseMtx(mtx.toMtxPtr());

        TVec3f trans;
        MR::extractMtxTrans(mtx.toMtxPtr(), &trans);

        setPlayerPos(trans);
    }
void setPlayerPosOnGroundAndWait(const char* pParam1) {
        TPos3f mtx;
        mtx.identity();

        MR::findNamePosOnGround(pParam1, mtx.toMtxPtr());
        MarioAccess::setBaseMtx(mtx.toMtxPtr());
        MarioAccess::getPlayerActor()->resetCondition();
        MarioAccess::setStateWait();
    }
bool isPlayerPointedBy2POnTriggerButton() {
        return MR::isStarPointerPointing2POnTriggerButton(MarioAccess::getPlayerActor(), "弱", true, false);
    }
void endBindAndPlayerDamage(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);
        info._20 |= 0xC0000000;
        info.mFlags.mDamageType = 1;

        MarioAccess::endRush(&info);
    }
void endBindAndPlayerFlip(LiveActor* pActor, const TVec3f& rParam2) {
        RushEndInfo info = RushEndInfo(pActor, 3, rParam2, true, 0);
        info._20 |= 0xC0000000;
        info.mFlags.mDamageType = 6;

        MarioAccess::endRush(&info);
    }
void endBindAndPlayerAcidDamage(LiveActor* pActor) {
        RushEndInfo info = RushEndInfo(pActor, 3, TVec3f(0.0f, 0.0f, 0.0f), true, 0);
        info._20 |= 0xC0000000;
        info.mFlags.mDamageType = 4;

        MarioAccess::endRush(&info);
    }
void endBindAndPlayerFreezeDamage(LiveActor* pActor) {
        RushEndInfo info = RushEndInfo(pActor, 3, TVec3f(0.0f, 0.0f, 0.0f), true, 0);
        info._20 |= 0xC0000000;
        info.mFlags.mDamageType = 3;

        MarioAccess::endRush(&info);
    }
void endBindAndPlayerFireDamage(LiveActor* pActor) {
        RushEndInfo info = RushEndInfo(pActor, 3, TVec3f(0.0f, 0.0f, 0.0f), true, 0);
        info._20 |= 0xC0000000;
        info.mFlags.mDamageType = 2;

        MarioAccess::endRush(&info);
    }
void endBindAndPlayerElectricDamage(LiveActor* pActor) {
        RushEndInfo info = RushEndInfo(pActor, 3, TVec3f(0.0f, 0.0f, 0.0f), true, 0);
        info._20 |= 0xC0000000;
        info.mFlags.mDamageType = 5;

        MarioAccess::endRush(&info);
    }
void endBindAndPlayerDamageMsg(LiveActor* pActor, u32 param2) {
        endBindAndPlayerDamageMsg(pActor, param2, TVec3f(0.0f, 0.0f, 0.0f));
    }
void endBindAndPlayerDamageMsg(LiveActor* pActor, u32 param2, const TVec3f& rParam3) {
        switch (param2) {
        case 0x57:
        case 0x58:
        case 0x59:
            endBindAndPlayerFireDamage(pActor);
            break;
        case 0x5C:
            endBindAndPlayerAcidDamage(pActor);
            break;
        case 0x5D:
            endBindAndPlayerFreezeDamage(pActor);
            break;
        case 0x5A:
        case 0x5B:
            endBindAndPlayerElectricDamage(pActor);
            break;
        case 0x56:
            endBindAndPlayerDamage(pActor, rParam3);
            break;
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x51:
        case 0x52:
            endBindAndPlayerWeakGravityLimitJump(pActor, rParam3);
            break;
        case 0x50:
            endBindAndPlayerFlip(pActor, rParam3);
            break;
        default:
            endBindAndPlayerDamage(pActor, rParam3);
            break;
        }
    }
void getPlayerFrontVec(TVec3f* pVec) {
        MarioAccess::getPlayerActor()->getFrontVec(pVec);
    }
void getPlayerSideVec(TVec3f* pVec) {
        MarioAccess::getPlayerActor()->getSideVec(pVec);
    }
void offPlayerControl() {
        MarioAccess::offControl();
    }
void onPlayerControl(bool param1) {
        MarioAccess::onControl(param1);
    }
void hidePlayer() {
        MarioAccess::hide();
    }
void showPlayer() {
        MarioAccess::show();
    }
void requestMovementOnPlayer() {
        requestMovementOn(MarioAccess::getPlayerActor());
    }
LiveActor* getCurrentRushActor() {
        if (isPlayerInRush() == false) {
            return nullptr;
        }

        return MarioAccess::getPlayerActor()->_924->mHost;
    }
f32 calcDistanceToPlayer(const TVec3f& rPos) {
        return getPlayerPos()->distance(rPos);
    }
void startSoundPlayerJ(const char* pName) {
        MarioAccess::getPlayerActor()->playSound(pName, -1);
    }
void startPlayerDownWipe() {
        MarioAccess::startDownWipe();
    }
bool isPlayerSquat() {
        return MarioAccess::isSquat();
    }
bool isPlayerHipDropFalling() {
        return MarioAccess::isHipDropFalling();
    }
void tryPlayerCoinPull() {
        MarioAccess::tryCoinPull();
    }
void endBindAndPlayerWait(LiveActor* pActor) {
        TVec3f velocity(0.0f, 0.0f, 0.0f);
        RushEndInfo info(pActor, 0, velocity, false, 0);

        MarioAccess::endRush(&info);
    }
void setBckRatePlayer(f32 rate) {
        MR::setBckRate(MarioAccess::getPlayerActor(), rate);
    }
void setPlayerUpperRotateY(f32 angle) {
        MarioAccess::getPlayerActor()->setUpperRotateY(angle);
    }
void endBindAndPlayerForceJump(LiveActor* pActor, const TVec3f& rParam2, u32 param3) {
        RushEndInfo info = RushEndInfo(pActor, 2, rParam2, true, param3);
        info._20 |= 0x40000000;

        MarioAccess::endRush(&info);
    }
void setBckBlendWeight(f32 param1, f32 param2) {
        const f32 blendWeight[] = {param1, param2};

        MarioAccess::setAnimationBlendWeight(blendWeight);
    }
void setBckBlendWeight(f32 param1, f32 param2, f32 param3, f32 param4) {
        const f32 blendWeight[] = {param1, param2, param3, param4};

        MarioAccess::setAnimationBlendWeight(blendWeight);
    }
f32 getBckFrameMaxPlayer() {
        return MarioAccess::getAnimationFrameMax();
    }
s16 getBckFrameMaxPlayer(const char* pName) {
        return MR::getBckFrameMax(MarioAccess::getPlayerActor(), pName);
    }
}
