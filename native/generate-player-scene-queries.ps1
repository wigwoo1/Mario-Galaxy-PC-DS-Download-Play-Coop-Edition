$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\PlayerUtil.cpp" -Raw
$result=@"
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
"@
foreach($signature in @('    LiveActor* getPlayerDemoActor(', '    bool isActorOnPlayer(', '    bool checkPlayerSwingTrigger(', 'void endBindAndPlayerForceWeakGravityJumpInputOff(', '    const TVec3f* getPlayerGroundNormal(', '    void pushPlayer(const TVec3f& rAccel)', '    void forceKillPlayerByGroundRace(', '    void startLevelSoundPlayer(', '    bool isPlayerTeresaDisappear(', '    f32 getPlayerHitRadius(', '    void pushPlayerFromArea(', 'bool isPlayerJumpRising(', 'void setPlayerFrontTargetVec(', 'void endBindAndPlayerWeakGravityJump(', 'void getPlayerGroundPos(', 'void tryPlayerPullActor(', 'void getPlayerTakePos(', 'void getPlayerThrowVec(', 'void endBindAndPlayerJumpWithRollLanding(', 'void initPlayerAfterOpeningDemo(', 'bool isPlayerHidden(', 'bool isPlayerOnPress(', 'void forceJumpPlayer(', 'bool isFpViewChangingFailure(', 'void resetPlayerStatus(', 'void forceKillPlayerByAbyss(', 'void startSoundPlayer(', 'bool isBckStoppedPlayer(', 'void tryPlayerKillTakingActor(', 'bool isOnPlayer(', 'bool isOffPlayerControl(', 'bool isPlayerRefuseTalk(', 'bool isPlayerElementModeNormal(', 'bool isPlayerDead(', 'bool isPlayerDamaging(', 'bool isPlayerInAreaObj(', 'void incPlayerLife(', 'TVec3f* getPlayerShadowRotate(', 'TVec3f* getPlayerCenterPos(', 'TVec3f* getPlayerRotate(', 'TVec3f* getPlayerVelocity(', 'const TVec3f* getPlayerGravity(', 'void getPlayerUpVec(', 'void getStarPiecePlayer(', 'void notifyDirectGetStarPiecePlayer(', 'void changePlayerItemStatus(', 'void incPlayerOxygen(', 'void stopSoundPlayer(', 'void setPlayerBaseMtx(', 'MtxPtr getPlayerBaseMtx(', 'void startBckPlayer(const char* pName, s32 param2)', 'void startBckPlayer(const char* pName, const char* pParam2)', 'void setPlayerSwingPermission(', 'bool isPlayerInBind(', 'bool isPlayerElementModeBee(', 'void startBckPlayer(const char* pName, const BckCtrlData& rCtrlData)', 'void startBckPlayerJ(', 'Triangle* getPlayerGroundingPolygon(', 'u16 getPlayerMovementTimer(', 'TVec3f* getPlayerLastMove(', 'bool isPlayerInWaterMode(', 'bool isPlayerOnWaterSurface(', 'bool isPlayerFlying(', 'void stopPlayerFpView(', 'void setCameraTargetToPlayer(', 'CubeCameraArea* getCameraCube(', 'bool isExistMario(', 'bool isPlayerElementModeHopper(', 'bool isPlayerElementModeTeresa(', 'bool isPlayerSwimming(', 'bool isOnGroundPlayer(', 'void forceKillPlayerByWaterRace(', 'void calcPlayerJointMtx(', 'bool isPlayerSwingAction(', 'bool isPlayerParalyzing(', 'bool isPlayerInRush(', 'bool isPlayerElementMode(', 'bool isPlayerDisableFpView(', 'bool isPlayerNeedBrakingCamera(', 'bool isPlayerConfrontDeath(', 'void endBindAndPlayerJump(', 'void endBindAndSpinDriverJump(', 'void endBindAndPlayerWeakGravityLimitJump(', 'bool isPlayerElementModeInvincible(', 'void setPlayerStateWait(', 'void setPlayerFrontVec(', 'void setPlayerSpot(', 'void setPlayerPos(const char* pParam1)', 'void setPlayerPosOnGroundAndWait(', 'bool isPlayerPointedBy2POnTriggerButton(', 'void endBindAndPlayerDamage(', 'void endBindAndPlayerFlip(', 'void endBindAndPlayerAcidDamage(', 'void endBindAndPlayerFreezeDamage(', 'void endBindAndPlayerFireDamage(', 'void endBindAndPlayerElectricDamage(', 'void endBindAndPlayerDamageMsg(LiveActor* pActor, u32 param2)', 'void endBindAndPlayerDamageMsg(LiveActor* pActor, u32 param2, const TVec3f& rParam3)', 'void getPlayerFrontVec(', 'void getPlayerSideVec(', 'void offPlayerControl(', 'void onPlayerControl(', 'void hidePlayer(', 'void showPlayer(', 'void requestMovementOnPlayer(', 'LiveActor* getCurrentRushActor(', 'f32 calcDistanceToPlayer(const TVec3f& rPos)', 'void startSoundPlayerJ(', 'void startPlayerDownWipe(', 'bool isPlayerSquat(', 'bool isPlayerHipDropFalling()', 'void tryPlayerCoinPull(', 'void endBindAndPlayerWait(', 'void setBckRatePlayer(', 'void setPlayerUpperRotateY(', 'void endBindAndPlayerForceJump(', 'void setBckBlendWeight(f32 param1, f32 param2)', 'void setBckBlendWeight(f32 param1, f32 param2, f32 param3, f32 param4)', 'f32 getBckFrameMaxPlayer()', 's16 getBckFrameMaxPlayer(const char* pName)')){
 $a=$s.IndexOf($signature);if($a -lt 0){throw "Missing original $signature"}
 $open=$s.IndexOf('{',$a);$depth=1;$end=$open+1
 while($depth){if($s[$end] -eq '{'){$depth++};if($s[$end] -eq '}'){$depth--};$end++}
 $result+="`r`n"+$s.Substring($a,$end-$a)
}
($result+"`r`n}")|Set-Content "$n\src\generated\PlayerSceneQueries.cpp"



































