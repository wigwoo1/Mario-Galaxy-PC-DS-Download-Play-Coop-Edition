#include <JSystem/JGeometry/TMatrix.hpp>
#include <math_types.hpp>
#include <Game/Util/FixedPosition.hpp>
#include <Game/Util/MtxUtil.hpp>
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


