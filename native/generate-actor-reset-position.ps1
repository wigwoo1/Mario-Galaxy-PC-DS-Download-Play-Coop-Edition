$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\ActorMovementUtil.cpp" -Raw
$out="#include `"Game/Util.hpp`"`r`n#include `"Game/LiveActor/LiveActor.hpp`"`r`n#include `"Game/LiveActor/Binder.hpp`"`r`n#include `"Game/LiveActor/HitSensor.hpp`"`r`n#include `"Game/LiveActor/ShadowController.hpp`"`r`n#include `"Game/Util/ActorShadowUtil.hpp`"`r`nnamespace MR {`r`n"
foreach($sig in @('    bool faceToPoint(MtxPtr', '    void addVelocityToGravityOrGround(', '    bool faceToVector(TQuat4f*', '    bool isFaceToPlayerHorizontalDegree(', '    void turnDirection(const LiveActor*', '    void turnDirectionToGround(', '    void moveAndTurnToDirection(LiveActor* pActor, TVec3f*', '    void moveAndTurnToTarget(LiveActor* pActor, TVec3f*', '    void moveAndTurnToPlayer(LiveActor* pActor, TVec3f*', '    void applyVelocityDampAndGravity(', '    void addVelocityMoveToTarget(LiveActor* pActor, const TVec3f& rTarget, f32 a3)', '    void addVelocityAwayFromTarget(LiveActor* pActor, const TVec3f& rTarget, f32 a3)', '    void addVelocityKeepHeight(LiveActor* pActor, const TVec3f& rA2, f32 a3, f32 a4, f32 a5)', '    bool reflectVelocityH(LiveActor* pActor, HitSensor* pSensor1, HitSensor* pSensor2, f32 a4)', '    f32 calcDistance(const HitSensor* pSensorA, const HitSensor* pSensorB, TVec3f* pOut)', '    void resetPosition(LiveActor* pActor)', '    void resetPosition(LiveActor* pActor, const TVec3f& rPosition)', '    void calcActorAxis(TVec3f* pAxisX, TVec3f* pAxisY, TVec3f* pAxisZ, const LiveActor* pActor)', '    void calcActorAxisX(TVec3f* pAxisX, const LiveActor* pActor)', '    void calcActorAxisY(TVec3f* pAxisY, const LiveActor* pActor)', '    void calcActorAxisZ(TVec3f* pAxisZ, const LiveActor* pActor)', '    void calcFrontVec(TVec3f* pFrontVec, const LiveActor* pActor)', '    void calcUpVec(TVec3f* pUpVec, const LiveActor* pActor)', '    void calcSideVec(TVec3f* pSideVec, const LiveActor* pActor)', '    bool isInDeath(const LiveActor* pActor, const TVec3f& rOffset)', '    void addVelocityToGravity(LiveActor* pActor, f32 a2)', '    void attenuateVelocity(LiveActor* pActor, f32 friction)', '    void zeroVelocity(LiveActor* pActor)', '    bool reboundVelocityFromCollision(LiveActor* pActor, f32 a2, f32 a3, f32 a4)', '    bool isInWater(const LiveActor* pActor, const TVec3f& rOffset)', '    void calcMtxFromGravityAndZAxis(TPos3f* pResult, const LiveActor* pActor, const TVec3f& rVGravity, const TVec3f& rZAxis)', '    bool addVelocityToCollisionNormal(LiveActor* pActor, f32 a2)', '    f32 calcDistance(const LiveActor* pActor, const TVec3f& rTrans)', '    void calcVelocityMoveToDirection(TVec3f* pA1, const LiveActor* pActor, const TVec3f& rA3, f32 a4)', '    void addVelocityMoveToDirection(LiveActor* pActor, const TVec3f& rA2, f32 a3)', '    void calcVelocityMoveToDirectionHorizon(TVec3f* pA1, const LiveActor* pActor, const TVec3f& rA3, f32 a4)', '    void addVelocity(LiveActor* pActor, const TVec3f& rVelocity)', '    void setVelocity(LiveActor* pActor, const TVec3f& rVelocity)', '    void turnDirectionDegree(const LiveActor* pActor, TVec3f* pA2, const TVec3f& rA3, f32 a4)', '    void rotateDirectionGravityDegree(const LiveActor* pActor, TVec3f* pA2, f32 angle)', '    bool turnDirectionToTargetUseGroundNormalDegree(const LiveActor* pActor, TVec3f* pA2, const TVec3f& rA3, f32 a4)', '    void turnDirectionToTargetDegree(const LiveActor* pActor, TVec3f* pA2, const TVec3f& rA3, f32 a4)', '    void turnDirectionToPlayerDegree(const LiveActor* pActor, TVec3f* pA2, f32 a3)', '    void makeQuatFromRotate(TQuat4f* pQuat, const LiveActor* pActor)', '    bool isFaceToPlayerDegreeHV(const LiveActor* pActor, const TVec3f& rA2, f32 angleH, f32 angleV)', '    bool isFaceToTargetHorizontalDegree(const LiveActor* pActor, const TVec3f& rA2, const TVec3f& rA3, f32 a4)', '    bool isInSightFanPlayer(const LiveActor* pActor, const TVec3f& rA2, f32 distance, f32 angleH, f32 angleV)', '    void addVelocityJump(LiveActor* pActor, f32 a2)', '    bool calcVelocityAreaMoveOnGround(TVec3f* pA1, const LiveActor* pActor)', '    bool calcVelocityRailMoveOnGround(TVec3f* pVelocity, const LiveActor* pActor)', '    bool calcVelocityAreaOrRailMoveOnGround(TVec3f* pA1, const LiveActor* pActor)', '    void setVelocityBlowAttack(LiveActor* pActor, HitSensor* pSensor1, HitSensor* pSensor2, f32 a4, f32 a5, s32 a6)', '    void addVelocityClockwiseToDirection(LiveActor* pActor, const TVec3f& rA2, f32 a3)', '    void addVelocityClockwiseToTarget(LiveActor* pActor, const TVec3f& rA2, f32 a3)', '    bool addVelocityLimit(LiveActor* pActor, const TVec3f& rA2)', '    bool sendMsgPushAndKillVelocityToTarget(LiveActor* pActor, HitSensor* pSensor1, HitSensor* pSensor2)', '    void addVelocityFromPush(LiveActor* pActor, f32 a2, HitSensor* pSensor1, HitSensor* pSensor2)', '    void makeQuatAndFrontFromRotate(TQuat4f* pQuat, TVec3f* pVec, const LiveActor* pActor)', '    void turnQuatUpToGravity(TQuat4f* pQuatDest, const TQuat4f& rPQuatSrc, const LiveActor* pActor)', '    void blendQuatFromGroundAndFront(TQuat4f* pDst, const LiveActor* pActor, const TVec3f& rFrontVec, f32 upVecBlendRate, f32 frontVecBlendRate)', '    void restrictVelocity(LiveActor* pActor, f32 speed)', '    bool reboundVelocityFromEachCollision(LiveActor* pActor, f32 a2, f32 a3, f32 a4, f32 a5)', '    void setVelocitySeparateHV(LiveActor* pActor, const TVec3f& rParam2, f32 velH, f32 velV)', '    void setVelocitySeparateHV(LiveActor* pActor, HitSensor* pSensor1, HitSensor* pSensor2, f32 a4, f32 a5)')) {
$a=$s.IndexOf($sig);if($a -lt 0){throw "Missing signature: $sig"}
$b=$s.IndexOf('{',$a);$depth=1;$i=$b+1
while($depth -gt 0 -and $i -lt $s.Length){if($s[$i] -eq '{'){$depth++};if($s[$i] -eq '}'){$depth--};$i++}
if($depth -ne 0){throw 'Unbalanced source body'}
$out+=$s.Substring($a,$i-$a)+"`r`n"
}
$out+="}`r`n"
$out=$out.Replace('PSVECScale(&toSensorB, pOut, 1.0f / distance);', '*pOut = toSensorB * (1.0f / distance);')
Set-Content "$n\src\generated\ActorResetPosition.cpp" $out






























