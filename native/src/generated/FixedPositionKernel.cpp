#include <JSystem/JGeometry/TMatrix.hpp>
#include <math_types.hpp>
#include "Game/Util/FixedPosition.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

FixedPosition::FixedPosition(const LiveActor* pActor, const char* pJointName, const TVec3f& rLocalTrans, const TVec3f& rLocalRotate) {
    init(MR::getJointMtx(pActor, pJointName), rLocalTrans, rLocalRotate);
}

FixedPosition::FixedPosition(const LiveActor* pActor, const TVec3f& rLocalTrans, const TVec3f& rLocalRotate) {
    init(pActor->getBaseMtx(), rLocalTrans, rLocalRotate);
}

FixedPosition::FixedPosition(MtxPtr pBaseMtx, const TVec3f& rLocalTrans, const TVec3f& rLocalRotate) {
    init(pBaseMtx, rLocalTrans, rLocalRotate);
}

void FixedPosition::calc() {
    mMtx.identity();
    mMtx.setRotate(mLocalRotate * PI_180);
    mMtx.setTrans(mLocalTrans);

    if (mBaseMtx != nullptr) {
        TMtx34f baseMtx;
        baseMtx.set(mBaseMtx);
        mMtx.concat(baseMtx, mMtx);
    }

    if (mNormalizeScale) {
        MR::makeMtxWithoutScale(&mMtx, mMtx);
    }
}

void FixedPosition::setBaseMtx(MtxPtr pBaseMtx) {
    mBaseMtx = pBaseMtx;
}

void FixedPosition::setLocalTrans(const TVec3f& rLocalTrans) {
    mLocalTrans.set(rLocalTrans);
}

void FixedPosition::init(MtxPtr pBaseMtx, const TVec3f& rLocalTrans, const TVec3f& rLocalRotate) {
    setBaseMtx(pBaseMtx);
    mLocalTrans.set(rLocalTrans);
    mLocalRotate.set(rLocalRotate);
    mMtx.identity();
    mNormalizeScale = true;
}

FixedPosition::FixedPosition(const LiveActor* pActor, const char* pResName, const LiveActor* pResActor) {
    if (pResActor == nullptr) {
        pResActor = pActor;
    }

    ResourceHolder* resourceHolder = MR::getResourceHolder(pResActor);
    JMapInfo* csv = MR::tryCreateCsvParser(resourceHolder, "%s.bcsv", pResName);

    const char* jointName = nullptr;
    TVec3f trans(0.0f, 0.0f, 0.0f);
    TVec3f rotate(0.0f, 0.0f, 0.0f);

    MR::getCsvDataStrOrNULL(&jointName, csv, "JointName", 0);
    Vec nativeTrans{trans.x,trans.y,trans.z}; MR::getCsvDataVec(&nativeTrans, csv, "Trans", 0); trans.set(nativeTrans.x,nativeTrans.y,nativeTrans.z);
    Vec nativeRotate{rotate.x,rotate.y,rotate.z}; MR::getCsvDataVec(&nativeRotate, csv, "Rotate", 0); rotate.set(nativeRotate.x,nativeRotate.y,nativeRotate.z);

    if (jointName == nullptr) {
        init(pActor->getBaseMtx(), trans, rotate);
    } else {
        init(MR::getJointMtx(pActor, jointName), trans, rotate);
    }
}

