#include <JSystem/JParticle/JPAFieldBlock.hpp>
#include <JSystem/JParticle/JPAEmitter.hpp>
#include <JSystem/JParticle/JPAParticle.hpp>
#include <Game/Util/MathUtil.hpp>
void JPAFieldAir::prepare(JPAEmitterWorkData* pWork, JPAFieldBlock* pBlock) {
    TVec3f direction(pBlock->getDir());
    MR::normalizeOrZero(&direction);
    if (pBlock->checkStatus(2)) {
        mAccel.scale(pBlock->getMag(), direction);
    } else {
        PSMTXMultVecSR(pWork->mRotationMtx, &direction, &mAccel);
        mAccel.scale(pBlock->getMag());
    }
}
void JPAFieldVortex::prepare(JPAEmitterWorkData* pWork, JPAFieldBlock* pBlock) {
    PSMTXMultVecSR(pWork->mGlobalRot, &pBlock->getDir(), &field_0x10);
    MR::normalizeOrZero(&field_0x10);
    field_0x1c = pBlock->getPos().z * pBlock->getPos().z;
    field_0x20 = 1.0f / field_0x1c;
}
void JPAFieldVortex::calc(JPAEmitterWorkData*, JPAFieldBlock* pBlock, JPABaseParticle* pParticle) {
    TVec3f radial;
    radial.scale(field_0x10.dot(pParticle->mLocalPosition), field_0x10);
    radial.sub(pParticle->mLocalPosition, radial);
    f32 distanceSquared = radial.squared();
    f32 magnitude;
    if (distanceSquared > field_0x1c) {
        magnitude = pBlock->getMagRndm();
    } else {
        f32 rate = distanceSquared * field_0x20;
        magnitude = (1.0f - rate) * pBlock->getMag() + rate * pBlock->getMagRndm();
    }

    MR::normalizeOrZero(&radial);
    mAccel.cross(radial, field_0x10);
    mAccel.scale(magnitude);
    calcAffect(pBlock, pParticle);
}
void JPAFieldConvection::prepare(JPAEmitterWorkData* pWork, JPAFieldBlock* pBlock) {
    TVec3f radial;
    TVec3f tangent;
    tangent.cross(pBlock->getPos(), pBlock->getDir());
    radial.cross(pBlock->getDir(), tangent);
    PSMTXMultVecSR(pWork->mGlobalRot, &radial, &field_0x10);
    PSMTXMultVecSR(pWork->mGlobalRot, &pBlock->getDir(), &field_0x1c);
    PSMTXMultVecSR(pWork->mGlobalRot, &tangent, &field_0x28);
    MR::normalizeOrZero(&field_0x10);
    MR::normalizeOrZero(&field_0x1c);
    MR::normalizeOrZero(&field_0x28);
}
void JPAFieldSpin::prepare(JPAEmitterWorkData* pWork, JPAFieldBlock* pBlock) {
    TVec3f axis;
    Mtx rotation;
    PSMTXMultVecSR(pWork->mGlobalRot, &pBlock->getDir(), &axis);
    MR::normalizeOrZero(&axis);
    PSMTXRotAxisRad(rotation, &axis, pBlock->getMag());
    field_0x10.set< f32 >(rotation[0][0], rotation[1][0], rotation[2][0]);
    field_0x1c.set< f32 >(rotation[0][1], rotation[1][1], rotation[2][1]);
    field_0x28.set< f32 >(rotation[0][2], rotation[1][2], rotation[2][2]);
}

