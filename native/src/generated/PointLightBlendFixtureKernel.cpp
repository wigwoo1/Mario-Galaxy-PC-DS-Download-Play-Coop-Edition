#include <Game/Map/LightPointCtrl.hpp>
#include <Game/Util/MathUtil.hpp>
namespace {constexpr s32 sDefaultBlendTime=30;constexpr f32 sDefaultDistRef=15;}
LightPointCtrl::LightPointCtrl()
    : mStep(-1), mBlendTime(::sDefaultBlendTime), mCurrentActor(), mPreviousActor(), mCandidateActor(), mCurrentInfo(),
      mTargetInfo(), mPreviousInfo() {
    mCurrentInfo = new PointLightInfo();
    mTargetInfo = new PointLightInfo();
    mPreviousInfo = new PointLightInfo();

    clearPointLight(mCurrentInfo);
    clearPointLight(mTargetInfo);
    clearPointLight(mPreviousInfo);
}
void LightPointCtrl::clearPointLight(PointLightInfo* pInfo) {
    Vec pos;
    pos.x = 0.0f;
    pos.y = 0.0f;
    pos.z = 0.0f;
    pInfo->mPos = pos;

    GXColor color;
    color.r = 0;
    color.g = 0;
    color.b = 0;
    color.a = 255;
    pInfo->mColor = color;

    pInfo->mRefBrightness = 0.001f;
    pInfo->mRefDistance = ::sDefaultDistRef;
    pInfo->mDistAttnFn = GX_DA_STEEP;
    mBlendTime = ::sDefaultBlendTime;
}
void LightPointCtrl::blendPointLight(PointLightInfo* pDst, const PointLightInfo& rStart, const PointLightInfo& rEnd, f32 t) {
    if (mCurrentActor == nullptr) {
        pDst->mPos = rStart.mPos;
    } else if (mPreviousActor == nullptr) {
        pDst->mPos = rEnd.mPos;
    } else {
        MR::blendVec(&pDst->mPos, rStart.mPos, rEnd.mPos, t);
    }

    f32 start = mPreviousActor == nullptr ? 0.95f : rStart.mRefBrightness;
    f32 end = mCurrentActor == nullptr ? 0.95f : rEnd.mRefBrightness;
    pDst->mRefBrightness = MR::getLinerValue(t, start, end, 1.0f);
    MR::blendColor(&pDst->mColor, rStart.mColor, rEnd.mColor, t);
    pDst->mRefDistance = MR::getLinerValue(t, rStart.mRefDistance, rEnd.mRefDistance, 1.0f);
}
