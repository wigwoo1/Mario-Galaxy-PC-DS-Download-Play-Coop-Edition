#include <JSystem/JParticle/JPAEmitterManager.hpp>
#include <JSystem/JParticle/JPAEmitter.hpp>
#include <Game/Util/MathUtil.hpp>
#include <JSystem/JParticle/JPAResource.hpp>
#include <JSystem/JParticle/JPADynamicsBlock.hpp>
#include <JSystem/JParticle/JPABaseShape.hpp>
void JPAEmitterManager::calcYBBCam() {
    TVec3f axis(0.0f, pWd->mPosCamMtx[1][1], pWd->mPosCamMtx[2][1]);
    if (!MR::isNearZero(axis)) {
        MR::normalize(&axis);
        pWd->mYBBCamMtx[0][0] = 1.0f;
        pWd->mYBBCamMtx[0][1] = 0.0f;
        pWd->mYBBCamMtx[0][2] = 0.0f;
        pWd->mYBBCamMtx[0][3] = pWd->mPosCamMtx[0][3];
        pWd->mYBBCamMtx[1][0] = 0.0f;
        pWd->mYBBCamMtx[1][1] = axis.y;
        pWd->mYBBCamMtx[1][2] = -axis.z;
        pWd->mYBBCamMtx[1][3] = pWd->mPosCamMtx[1][3];
        pWd->mYBBCamMtx[2][0] = 0.0f;
        pWd->mYBBCamMtx[2][1] = axis.z;
        pWd->mYBBCamMtx[2][2] = axis.y;
        pWd->mYBBCamMtx[2][3] = pWd->mPosCamMtx[2][3];
    }
}
void JPABaseEmitter::init(JPAEmitterManager* pManager, JPAResource* pResource) {
    mpEmtrMgr = pManager;
    mpRes = pResource;
    const JPADynamicsBlockData* pData = mpRes->getDyn()->mpData;
    mLocalScl.set< f32 >(pData->mEmitterScl.x, pData->mEmitterScl.y, pData->mEmitterScl.z);
    pData = mpRes->getDyn()->mpData;
    mLocalTrs.set< f32 >(pData->mEmitterTrs.x, pData->mEmitterTrs.y, pData->mEmitterTrs.z);
    pData = mpRes->getDyn()->mpData;
    mLocalDir.set< f32 >(pData->mEmitterDir.x, pData->mEmitterDir.y, pData->mEmitterDir.z);
    if (!MR::isNearZero(mLocalDir)) {
        MR::normalize(&mLocalDir);
    }

    const JGeometry::TVec3< s16 >& rRotation = mpRes->getDyn()->mpData->mEmitterRot;
    const s16 x = rRotation.x;
    const s16 z = rRotation.z;
    const s16 y = rRotation.y;
    mLocalRot.set(x, y, z);
    mMaxFrame = mpRes->getDyn()->getMaxFrame();
    mLifeTime = mpRes->getDyn()->getLifetime();
    mVolumeSize = mpRes->getDyn()->getVolumeSize();
    mRate = mpRes->getDyn()->getRate();
    mRateStep = mpRes->getDyn()->getRateStep();
    mVolumeSweep = mpRes->getDyn()->getVolumeSweep();
    mVolumeMinRad = mpRes->getDyn()->getVolumeMinRad();
    mAwayFromCenterSpeed = mpRes->getDyn()->getInitVelOmni();
    mAwayFromAxisSpeed = mpRes->getDyn()->getInitVelAxis();
    mDirSpeed = mpRes->getDyn()->getInitVelDir();
    mSpread = mpRes->getDyn()->getInitVelDirSp();
    mRndmDirSpeed = mpRes->getDyn()->getInitVelRndm();
    mAirResist = mpRes->getDyn()->getAirRes();
    mRndm.set_seed(mpEmtrMgr->pWd->mRndm.get_rndm_u());
    PSMTXIdentity(mGlobalRot);
    mGlobalScl.set(1.0f, 1.0f, 1.0f);
    mGlobalTrs.zero();
    mGlobalPScl.set(1.0f, 1.0f);
    mGlobalPrmClr.r = mGlobalPrmClr.g = mGlobalPrmClr.b = mGlobalPrmClr.a = mGlobalEnvClr.r = mGlobalEnvClr.g = mGlobalEnvClr.b = mGlobalEnvClr.a =
        255;
    pResource->getBsp()->getPrmClr(&mPrmClr);
    pResource->getBsp()->getEnvClr(&mEnvClr);
    mpUserWork = 0;
    mScaleOut = 1.0f;
    mEmitCount = 0.0f;
    mStatus = 0x30;
    mDrawTimes = 1;
    mTick = 0;
    mWaitTime = 0;
    mRateStepTimer = 0;
    mTexAnmIdx = 0;
}
