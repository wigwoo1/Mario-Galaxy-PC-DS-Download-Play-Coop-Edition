#include "Game/Util.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Binder.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ShadowController.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
namespace MR {
    bool faceToPoint(MtxPtr pMtx, TVec3f a2, f32 a3) {
        a2.sub(TVec3f(pMtx[0][3], pMtx[1][3], pMtx[2][3]));

        return faceToVector(pMtx, a2, a3);
    }
    void addVelocityToGravityOrGround(LiveActor* pActor, f32 a2) {
        if (isBindedGround(pActor)) {
            pActor->mVelocity.scaleAdd(-a2, *getGroundNormal(pActor), pActor->mVelocity);
        } else {
            addVelocityToGravity(pActor, a2);
        }
    }
    bool faceToVector(TQuat4f* pQuat, TVec3f a2, f32 a3) {
        TVec3f yDir;
        pQuat->getYDir(yDir);
        normalizeOrZero(&a2);
        if (vecKillElement(a2, yDir, &a2) > 0.95f) {
            return false;
        } else {
            return turnQuatZDirRad(pQuat, *pQuat, a2, a3 * PI_180);
        }
    }
    bool isFaceToPlayerHorizontalDegree(const LiveActor* pActor, const TVec3f& rA2, f32 a3) {
        return MR::isFaceToTargetHorizontalDegree(pActor, *MR::getPlayerPos(), rA2, a3);
    }
    void turnDirection(const LiveActor* pActor, TVec3f* pA2, const TVec3f& rA3, f32 a4) {
        turnVecToVecCosOnPlane(pA2, rA3, (pActor->mGravity), a4);
    }
    void turnDirectionToGround(const LiveActor* pActor, TVec3f* pA2) {
        TVec3f stack_8;
        if (isBindedGround(pActor)) {
            stack_8.scale(-1.0f, *getGroundNormal(pActor));
        } else {
            stack_8.set(pActor->mGravity);
        }

        pA2->orthogonalize(stack_8);
        normalize(pA2);
    }
    void moveAndTurnToDirection(LiveActor* pActor, TVec3f* pA2, const TVec3f& rA3, f32 a4, f32 a5, f32 a6, f32 a7) {
        turnVecToVecCosOnPlane(pA2, rA3, pActor->mGravity, cosDegree(a7));
        turnDirectionToGround(pActor, pA2);
        TVec3f stack_8;
        calcVelocityMoveToDirection(&stack_8, pActor, *pA2, a4);
        pActor->mVelocity.add(stack_8);
        pActor->mVelocity.scale(a6);
        reboundVelocityFromCollision(pActor, 0.0f, 0.0f, 1.0f);
        addVelocityToGravityOrGround(pActor, a5);
    }
    void moveAndTurnToTarget(LiveActor* pActor, TVec3f* pA2, const TVec3f& rA3, f32 a4, f32 a5, f32 a6, f32 a7) {
        TVec3f stack_8 = rA3 - pActor->mPosition;
        moveAndTurnToDirection(pActor, pA2, stack_8, a4, a5, a6, a7);
    }
    void moveAndTurnToPlayer(LiveActor* pActor, TVec3f* pA2, f32 a3, f32 a4, f32 a5, f32 a6) {
        TVec3f stack_8 = *getPlayerPos() - pActor->mPosition;
        moveAndTurnToDirection(pActor, pA2, stack_8, a3, a4, a5, a6);
    }
    void applyVelocityDampAndGravity(LiveActor* pActor, f32 a2, f32 groundedScalar, f32 airborneScalar, f32 fallingScalar, f32 a6) {
        TVec3f stack_38;
        TVec3f stack_2c;
        if (!isBindedGround(pActor)) {
            pActor->mVelocity.add(pActor->mGravity * a2);
        }

        TVec3f* pVelocity = &pActor->mVelocity;
        TVec3f* pGravThenVel = &pActor->mGravity;
        stack_38.killElement(*pVelocity, *pGravThenVel);
        f32 along = pActor->mGravity.dot(*pVelocity);
        stack_2c.scale(along, *pGravThenVel);
        if (isOnGround(pActor)) {
            stack_38.scale(groundedScalar);
        } else {
            stack_38.scale(airborneScalar);
        }

        if (stack_2c.dot(pActor->mGravity) < 0.0f) {
            stack_2c.scale(fallingScalar);
        }

        pActor->mVelocity.set(stack_38 + stack_2c);
        if (isOnGround(pActor)) {
            TVec3f stack_20(*getGroundNormal(pActor));
            TVec3f* pVelocity2 = &pActor->mVelocity;
            stack_38.killElement(*pVelocity2, stack_20);
            if (stack_38.squared() < a6 * a6) {
                pVelocity2->scale(stack_20.dot(pActor->mVelocity), stack_20);
            }
        }
    }
    void addVelocityMoveToTarget(LiveActor* pActor, const TVec3f& rTarget, f32 a3) {
        TVec3f toTarget = rTarget - pActor->mPosition;
        TVec3f accel;
        calcVelocityMoveToDirection(&accel, pActor, toTarget, a3);

        addVelocity(pActor, accel);
    }
    void addVelocityAwayFromTarget(LiveActor* pActor, const TVec3f& rTarget, f32 a3) {
        TVec3f fromTarget = pActor->mPosition - rTarget;

        TVec3f accel;
        calcVelocityMoveToDirection(&accel, pActor, fromTarget, a3);

        addVelocity(pActor, accel);
    }
    void addVelocityKeepHeight(LiveActor* pActor, const TVec3f& rA2, f32 a3, f32 a4, f32 a5) {
        TVec3f stack_8 = rA2 - pActor->mPosition;
        addVelocityToGravity(pActor, a4 * normalizeAbs((pActor->mGravity).dot(stack_8) - a3, -a5, a5));
    }
    bool reflectVelocityH(LiveActor* pActor, HitSensor* pSensor1, HitSensor* pSensor2, f32 a4) {
        TVec3f stack_14;
        calcSensorHorizonNormalize(&stack_14, (pActor->mGravity), pSensor2, pSensor1);
        a4 = (1.0f + a4) * pActor->mVelocity.dot(stack_14);
        if (a4 > 0.0f) {
            return false;
        }

        pActor->mVelocity.sub(stack_14 * a4);
        return true;
    }
    f32 calcDistance(const HitSensor* pSensorA, const HitSensor* pSensorB, TVec3f* pOut) {
        TVec3f toSensorB = pSensorB->mPosition - pSensorA->mPosition;

        f32 distance = toSensorB.length();

        if (pOut != nullptr) {
            if (distance > 0.0f) {
                *pOut = toSensorB * (1.0f / distance);
            } else {
                pOut->zero();
            }
        }

        return distance;
    }
    void resetPosition(LiveActor* pActor) {
        MR::clearHitSensors(pActor);

        if (pActor->mBinder != nullptr) {
            pActor->mBinder->clear();
        }

        if (MR::isCalcGravity(pActor)) {
            MR::calcGravity(pActor);
        }

        MR::calcAnimDirect(pActor);

        if (pActor->mCollisionParts != nullptr) {
            MR::resetAllCollisionMtx(pActor);
        }

        MR::requestCalcActorShadowAppear(pActor);
    }
    void resetPosition(LiveActor* pActor, const TVec3f& rPosition) {
        pActor->mPosition.set(rPosition);

        MR::resetPosition(pActor);
    }
    void calcActorAxis(TVec3f* pAxisX, TVec3f* pAxisY, TVec3f* pAxisZ, const LiveActor* pActor) {
        TRot3f mtx;
        mtx.identity();
        makeMtxTR(mtx, pActor);

        mtx.getXDir(*pAxisX);
        mtx.getYDir(*pAxisY);
        mtx.getZDir(*pAxisZ);
    }
    void calcActorAxisX(TVec3f* pAxisX, const LiveActor* pActor) {
        TRot3f mtx;
        mtx.identity();
        makeMtxTR(mtx, pActor);

        mtx.getXDir(*pAxisX);
    }
    void calcActorAxisY(TVec3f* pAxisY, const LiveActor* pActor) {
        TRot3f mtx;
        mtx.identity();
        makeMtxTR(mtx, pActor);

        mtx.getYDir(*pAxisY);
    }
    void calcActorAxisZ(TVec3f* pAxisZ, const LiveActor* pActor) {
        TRot3f mtx;
        mtx.identity();
        makeMtxTR(mtx, pActor);

        mtx.getZDir(*pAxisZ);
    }
    void calcFrontVec(TVec3f* pFrontVec, const LiveActor* pActor) {
        MtxPtr mtx = pActor->getBaseMtx();
        pFrontVec->set< f32 >(mtx[0][2], mtx[1][2], mtx[2][2]);
    }
    void calcUpVec(TVec3f* pUpVec, const LiveActor* pActor) {
        MtxPtr mtx = pActor->getBaseMtx();
        pUpVec->set< f32 >(mtx[0][1], mtx[1][1], mtx[2][1]);
    }
    void calcSideVec(TVec3f* pSideVec, const LiveActor* pActor) {
        MtxPtr mtx = pActor->getBaseMtx();
        pSideVec->set< f32 >(mtx[0][0], mtx[1][0], mtx[2][0]);
    }
    bool isInDeath(const LiveActor* pActor, const TVec3f& rOffset) {
        return MR::isInDeath(pActor->mPosition + rOffset);
    }
    void addVelocityToGravity(LiveActor* pActor, f32 a2) {
        addVelocity(pActor, pActor->mGravity * a2);
    }
    void attenuateVelocity(LiveActor* pActor, f32 friction) {
        pActor->mVelocity.scale(friction);
    }
    void zeroVelocity(LiveActor* pActor) {
        pActor->mVelocity.zero();
    }
    bool reboundVelocityFromCollision(LiveActor* pActor, f32 a2, f32 a3, f32 a4) {
        if (!isBinded(pActor)) {
            return false;
        }

        TVec3f bindedReactnVec = *getBindedFixReactionVector(pActor);
        if (isNearZero(bindedReactnVec)) {
            return false;
        }

        normalize(&bindedReactnVec);
        f32 f31 = bindedReactnVec.dot(pActor->mVelocity);
        if (f31 < -a3) {
            pActor->mVelocity.sub(bindedReactnVec * f31);
            pActor->mVelocity.scale(a4);
            pActor->mVelocity.sub(bindedReactnVec * f31 * a2);
            return true;
        }

        if (f31 < 0.0f) {
            pActor->mVelocity.sub(bindedReactnVec * f31);
            return false;
        }

        return false;
    }
    bool isInWater(const LiveActor* pActor, const TVec3f& rOffset) {
        return MR::isInWater(pActor->mPosition + rOffset);
    }
    void calcMtxFromGravityAndZAxis(TPos3f* pResult, const LiveActor* pActor, const TVec3f& rVGravity, const TVec3f& rZAxis) {
        TPos3f pos;
        pos.setInline(pActor->getBaseMtx());
        TVec3f yDir;
        TVec3f zDir;
        TVec3f xDir;
        TVec3f xDir2;
        if (!isNearZero(rVGravity)) {
            TVec3f stack_8 = -rVGravity;
            yDir = stack_8;
        } else {
            pos.getYDir(yDir);
        }

        if (!isNearZero(rZAxis)) {
            zDir = rZAxis;
        } else {
            pos.getZDir(zDir);
        }

        xDir.cross(zDir, yDir);
        if (isNearZero(xDir)) {
            pos.getXDir(xDir2);
            zDir.cross(xDir2, yDir);
        }

        makeMtxUpFrontPos(pResult, yDir, zDir, pActor->mPosition);
    }
    bool addVelocityToCollisionNormal(LiveActor* pActor, f32 a2) {
        if (isBinded(pActor) == false) {
            return false;
        }

        TVec3f stack_14(*getBindedFixReactionVector(pActor));

        if (isNearZero(stack_14)) {
            return false;
        }

        normalize(&stack_14);
        (pActor->mVelocity).add(stack_14 * a2);

        return true;
    }
    f32 calcDistance(const LiveActor* pActor, const TVec3f& rTrans) {
        return pActor->mPosition.distance(rTrans);
    }
    void calcVelocityMoveToDirection(TVec3f* pA1, const LiveActor* pActor, const TVec3f& rA3, f32 a4) {
        calcVelocityMoveToDirectionHorizon(pA1, pActor, rA3, a4);

        if (isOnGround(pActor)) {
            pA1->orthogonalize(*getGroundNormal(pActor));
        }
    }
    void addVelocityMoveToDirection(LiveActor* pActor, const TVec3f& rA2, f32 a3) {
        TVec3f accel;
        calcVelocityMoveToDirection(&accel, pActor, rA2, a3);

        addVelocity(pActor, accel);
    }
    void calcVelocityMoveToDirectionHorizon(TVec3f* pA1, const LiveActor* pActor, const TVec3f& rA3, f32 a4) {
        pA1->killElement(rA3, pActor->mGravity);
        normalizeOrZero(pA1);
        pA1->scale(a4);
    }
    void addVelocity(LiveActor* pActor, const TVec3f& rVelocity) {
        pActor->mVelocity.add(rVelocity);
    }
    void setVelocity(LiveActor* pActor, const TVec3f& rVelocity) {
        pActor->mVelocity.set(rVelocity);
    }
    void turnDirectionDegree(const LiveActor* pActor, TVec3f* pA2, const TVec3f& rA3, f32 a4) {
        turnVecToVecCosOnPlane(pA2, rA3, (pActor->mGravity), cosDegree(a4));
    }
    void rotateDirectionGravityDegree(const LiveActor* pActor, TVec3f* pA2, f32 angle) {
        rotateVecRadian(pA2, (pActor->mGravity), PI_180 * angle);
    }
    bool turnDirectionToTargetUseGroundNormalDegree(const LiveActor* pActor, TVec3f* pA2, const TVec3f& rA3, f32 a4) {
        TVec3f stack_8 = rA3 - pActor->mPosition;
        a4 = cosDegree(a4);
        return turnVecToVecCosOnPlane(pA2, stack_8, isBindedGround(pActor) ? *getGroundNormal(pActor) : (pActor->mGravity), a4);
    }
    void turnDirectionToTargetDegree(const LiveActor* pActor, TVec3f* pA2, const TVec3f& rA3, f32 a4) {
        TVec3f stack_8 = rA3 - pActor->mPosition;
        turnVecToVecCosOnPlane(pA2, stack_8, (pActor->mGravity), cosDegree(a4));
    }
    void turnDirectionToPlayerDegree(const LiveActor* pActor, TVec3f* pA2, f32 a3) {
        turnDirectionToTargetDegree(pActor, pA2, *getPlayerPos(), a3);
    }
    void makeQuatFromRotate(TQuat4f* pQuat, const LiveActor* pActor) {
        makeQuatRotateDegree(pQuat, pActor->mRotation);
    }
    bool isFaceToPlayerDegreeHV(const LiveActor* pActor, const TVec3f& rA2, f32 angleH, f32 angleV) {
        return MR::isNearAngleDegreeHV(*MR::getPlayerPos() - pActor->mPosition, rA2, pActor->mGravity, angleH, angleV);
    }
    bool isFaceToTargetHorizontalDegree(const LiveActor* pActor, const TVec3f& rA2, const TVec3f& rA3, f32 a4) {
        TVec3f stack_20 = (rA2 - pActor->mPosition).killElement(pActor->mGravity);
        TVec3f stack_14 = rA3.killElement(pActor->mGravity);

        return MR::isNearAngleDegree(stack_20, stack_14, a4);
    }
    bool isInSightFanPlayer(const LiveActor* pActor, const TVec3f& rA2, f32 distance, f32 angleH, f32 angleV) {
        return isNearPlayerAnyTime(pActor, distance) && isFaceToPlayerDegreeHV(pActor, rA2, angleH, angleV);
    }
    void addVelocityJump(LiveActor* pActor, f32 a2) {
        addVelocity(pActor, pActor->mGravity * -a2);
    }
    bool calcVelocityAreaMoveOnGround(TVec3f* pA1, const LiveActor* pActor) {
        pA1->zero();
        if ((isOnGround(pActor) == false) || (isBindedGroundAreaMove(pActor) == false)) {
            return false;
        }

        calcAreaMoveVelocity(pA1, pActor->mPosition);
        return true;
    }
    bool calcVelocityRailMoveOnGround(TVec3f* pVelocity, const LiveActor* pActor) {
        pVelocity->zero();
        if (!isOnGround(pActor) || !isBindedGroundRailMove(pActor)) {
            return false;
        }

        LiveActor* pGroundActor = getGroundSensor(pActor)->mHost;
        isExistRail(pGroundActor);
        calcNearestRailDirection(pVelocity, pGroundActor, pActor->mPosition);
        s32 railArg = -1;
        getRailArg3NoInit(pGroundActor, &railArg);
        pVelocity->scale(railArg == -1 ? 10 : railArg);
        return true;
    }
    bool calcVelocityAreaOrRailMoveOnGround(TVec3f* pA1, const LiveActor* pActor) {
        return calcVelocityAreaMoveOnGround(pA1, pActor) || calcVelocityRailMoveOnGround(pA1, pActor);
    }
    void setVelocityBlowAttack(LiveActor* pActor, HitSensor* pSensor1, HitSensor* pSensor2, f32 a4, f32 a5, s32 a6) {
        setVelocitySeparateHV(pActor, pSensor1, pSensor2, a4, a5);

        if (isNearPlayerAnyTime(pActor, 1000.0f)) {
            stopSceneForDefaultHit(a6);
        }
    }
    void addVelocityClockwiseToDirection(LiveActor* pActor, const TVec3f& rA2, f32 a3) {
        TVec3f stack_14;
        if (normalizeOrZero(rA2, &stack_14)) {
            return;
        }

        stack_14.cross(pActor->mGravity, stack_14);
        TVec3f stack_8;
        calcVelocityMoveToDirection(&stack_8, pActor, stack_14, a3);
        (pActor->mVelocity).add(stack_8);
    }
    void addVelocityClockwiseToTarget(LiveActor* pActor, const TVec3f& rA2, f32 a3) {
        addVelocityClockwiseToDirection(pActor, rA2 - pActor->mPosition, a3);
    }
    bool addVelocityLimit(LiveActor* pActor, const TVec3f& rA2) {
        TVec3f stack_18;
        f32 stack_8;
        separateScalarAndDirection(&stack_8, &stack_18, rA2);

        if (isNearZero(stack_18)) {
            return false;
        }

        f32 f1 = stack_18.dot(pActor->mVelocity);

        if (stack_8 <= f1) {
            return false;
        }

        pActor->mVelocity.add(stack_18 * (stack_8 - f1));

        return true;
    }
    bool sendMsgPushAndKillVelocityToTarget(LiveActor* pActor, HitSensor* pSensor1, HitSensor* pSensor2) {
        if (sendMsgPush(pSensor1, pSensor2)) {
            TVec3f stack_8 = pSensor2->mPosition - pSensor1->mPosition;
            normalizeOrZero(&stack_8);
            if (pActor->mVelocity.dot(stack_8) < 0.0f) {
                TVec3f* pVelocity = &pActor->mVelocity;
                pActor->mVelocity.killElement(*pVelocity, stack_8);
            }

            return true;
        }

        return false;
    }
    void addVelocityFromPush(LiveActor* pActor, f32 a2, HitSensor* pSensor1, HitSensor* pSensor2) {
        TVec3f stack_14 = pSensor2->mPosition - pSensor1->mPosition;
        if (a2 < (pActor->mVelocity).dot(-pActor->mGravity)) {
            vecKillElement(stack_14, pActor->mGravity, &stack_14);
        }

        stack_14.setLength(a2);
        addVelocityLimit(pActor, stack_14);
    }
    void makeQuatAndFrontFromRotate(TQuat4f* pQuat, TVec3f* pVec, const LiveActor* pActor) {
        makeQuatRotateDegree(pQuat, pActor->mRotation);
        pQuat->getZDir(*pVec);
    }
    void turnQuatUpToGravity(TQuat4f* pQuatDest, const TQuat4f& rPQuatSrc, const LiveActor* pActor) {
        TVec3f yDir;
        rPQuatSrc.getYDir(yDir);

        TQuat4f q1;
        if (yDir.dot(-pActor->mGravity) <= -0.999f) {
            TVec3f xDir;
            rPQuatSrc.getXDir(xDir);
            q1.setRotate(xDir, PI);
        } else {
            q1.setRotate(yDir, -pActor->mGravity);
        }

        PSQUATMultiply(reinterpret_cast< const Quaternion* >(&q1), reinterpret_cast< const Quaternion* >(&rPQuatSrc),
                       reinterpret_cast< Quaternion* >(pQuatDest));
        pQuatDest->normalize();
    }
    void blendQuatFromGroundAndFront(TQuat4f* pDst, const LiveActor* pActor, const TVec3f& rFrontVec, f32 upVecBlendRate, f32 frontVecBlendRate) {
        MR::blendQuatUpFront(pDst, MR::isBindedGround(pActor) ? *MR::getGroundNormal(pActor) : -pActor->mGravity, rFrontVec, upVecBlendRate,
                             frontVecBlendRate);
    }
    void restrictVelocity(LiveActor* pActor, f32 speed) {
        if (pActor->mVelocity.squared() > speed * speed) {
            pActor->mVelocity.setLength(speed);
        }
    }
    bool reboundVelocityFromEachCollision(LiveActor* pActor, f32 a2, f32 a3, f32 a4, f32 a5) {
        if (!isBinded(pActor)) {
            return false;
        }

        TVec3f bindedReactnVec = *getBindedFixReactionVector(pActor);
        if (isNearZero(bindedReactnVec)) {
            return false;
        }

        normalize(&bindedReactnVec);
        f32 f31 = bindedReactnVec.dot(pActor->mGravity);
        f32 f30 = 0.0f;
        if (isFloorPolygon(f31)) {
            f30 = a2;
        } else if (isWallPolygon(f31)) {
            f30 = a3;
        } else if (isCeilingPolygon(f31)) {
            f30 = a4;
        }

        f31 = bindedReactnVec.dot(pActor->mVelocity);
        f32 f0 = -a5;
        if (f31 < f0) {
            pActor->mVelocity.sub(bindedReactnVec * f31 * (1.0f + f30));
            return true;
        }

        if (f31 < 0.0f) {
            pActor->mVelocity.sub(bindedReactnVec * f31);
            return false;
        }

        return false;
    }
    void setVelocitySeparateHV(LiveActor* pActor, const TVec3f& rParam2, f32 velH, f32 velV) {
        TVec3f sideVec;
        sideVec.killElement(rParam2, pActor->mGravity);
        normalizeOrZero(&sideVec);

        setVelocity(pActor, sideVec * velH - pActor->mGravity * velV);
    }
    void setVelocitySeparateHV(LiveActor* pActor, HitSensor* pSensor1, HitSensor* pSensor2, f32 a4, f32 a5) {
        setVelocitySeparateHV(pActor, pSensor2->mPosition - pSensor1->mPosition, a4, a5);
    }
}

