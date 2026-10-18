#include <revolution/gx/GXLighting.h>
#include <Game/Map/LightFunction.hpp>
#include <Game/Map/LightPointCtrl.hpp>
#include <Game/Util/CameraUtil.hpp>
#include <Game/Util/Color.hpp>
#include <Game/Util/MathUtil.hpp>
void LightFunction::initLightRegisterAll() {
    const GXLightID cLightDataIDs[8] = {GX_LIGHT0, GX_LIGHT1, GX_LIGHT2, GX_LIGHT3, GX_LIGHT4, GX_LIGHT5, GX_LIGHT6, GX_LIGHT7};
    Color8 c(255, 255, 255, 255);

    for (s32 i = 0; i < 8; i++) {
        GXLightObj obj{};
        GXInitLightColor(&obj, c);
        GXInitLightPos(&obj, 0.0f, 0.0f, 0.0f);
        GXInitLightDir(&obj, 0.0f, -1.0f, 0.0f);
        GXInitLightAttn(&obj, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
        GXInitSpecularDir(&obj, 0.0f, -1.0f, 0.0f);
        GXInitSpecularDirHA(&obj, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f);
        GXInitLightAttn(&obj, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
        GXLoadLightObjImm(&obj, cLightDataIDs[i]);
    }
}
namespace {
    void blendActorLightPos(const LightInfo& rFrom, const LightInfo& rTo, LightInfo* pResult, f32 rate) {
        if (pResult->mIsFollowCamera) {
            if (rFrom.mIsFollowCamera) {
                MR::blendVec(&pResult->mPos, rFrom.mPos, rTo.mPos, rate);
            } else {
                TVec3f pos(rFrom.mPos);
                MR::getCameraViewMtx().mult(pos, pos);
                MR::blendVec(&pResult->mPos, pos, rTo.mPos, rate);
            }
        } else if (rFrom.mIsFollowCamera) {
            TVec3f pos(rFrom.mPos);
            MR::getCameraInvViewMtx().mult(pos, pos);
            MR::blendVec(&pResult->mPos, pos, rTo.mPos, rate);
        } else {
            MR::blendVec(&pResult->mPos, rFrom.mPos, rTo.mPos, rate);
        }
    }
    void loadLightDiffuse(GXColor color, const TVec3f& rPos, GXLightID lightID) {
        GXLightObj lightObj{};

        GXInitLightPos(&lightObj, rPos.x, rPos.y, rPos.z);
        GXInitLightAttn(&lightObj, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);

        GXColor new_color(color);
        GXInitLightColor(&lightObj, new_color);
        GXLoadLightObjImm(&lightObj, lightID);
    }
    inline void loadCameraLight(const LightInfo& rInfo, GXLightID lightID) {
        TVec3f position(rInfo.mPos);
        ::loadLightDiffuse(GXColor(rInfo.mColor), position, lightID);
    }
    void loadLightInfoDiffuse(const LightInfo& rInfo, GXLightID lightID) {
        if (rInfo.mIsFollowCamera) {
            ::loadCameraLight(rInfo, lightID);
        } else {
            TVec3f v14(rInfo.mPos);
            MR::getCameraViewMtx().mult(v14, v14);
            ::loadLightDiffuse(GXColor(rInfo.mColor), v14, lightID);
        }
    }
}
void LightFunction::loadActorLightInfo(const ActorLightInfo* pInfo) {
    ::loadLightInfoDiffuse(pInfo->mInfo0, GX_LIGHT0);
    ::loadLightInfoDiffuse(pInfo->mInfo1, GX_LIGHT1);

    u8 alpha = pInfo->mAlpha2;
    GXColor c = {0, 0, 0, alpha};
    ::loadLightDiffuse(c, TVec3f(0.0f, 0.0f, 0.0f), GX_LIGHT2);
    GXSetChanAmbColor(GX_COLOR0A0, GXColor(pInfo->mColor));
}
void LightFunction::loadPointLightInfo(const PointLightInfo* pInfo) {
    TVec3f v7(pInfo->mPos);
    MR::getCameraViewMtx().mult(v7, v7);
    GXLightObj obj{};
    GXInitLightPos(&obj, v7.x, v7.y, v7.z);
    GXInitLightDistAttn(&obj, pInfo->mRefDistance, pInfo->mRefBrightness, pInfo->mDistAttnFn);
    GXInitLightSpot(&obj, 0.0, GX_SP_OFF);
    GXInitLightColor(&obj, pInfo->mColor);
    GXLoadLightObjImm(&obj, GX_LIGHT4);
}
void LightFunction::loadLightInfoCoin(const LightInfoCoin* pInfo) {
    ::loadLightInfoDiffuse(pInfo->base, GX_LIGHT0);
    TVec3f v7(0.0f, 0.0f, -1.0f);
    GXLightObj obj{};
    f32 v = pInfo->_18;
    GXInitLightColor(&obj, GXColor(pInfo->_14));
    GXInitSpecularDir(&obj, v7.x, v7.y, v7.z);
    GXInitLightAttn(&obj, 0.0f, 0.0f, 1.0f, (v / 2.0f), 0.0f, (1.0f - (v / 2.0f)));
    GXLoadLightObjImm(&obj, GX_LIGHT3);
}
void LightFunction::loadAllLightWhite() {
    const GXLightID cLightDataIDs[8] = {GX_LIGHT0, GX_LIGHT1, GX_LIGHT2, GX_LIGHT3, GX_LIGHT4, GX_LIGHT5, GX_LIGHT6, GX_LIGHT7};

    for (u32 i = 0; i < ARRAY_SIZE(cLightDataIDs); i++) {
        GXLightObj lightObj{};
        GXInitLightPos(&lightObj, 0.0f, 0.0f, 0.0f);

        GXColor color = {255, 255, 255, 255};
        GXInitLightColor(&lightObj, color);
        GXLoadLightObjImm(&lightObj, cLightDataIDs[i]);
    }
}
void LightFunction::blendActorLightInfo(ActorLightInfo* pResult, const ActorLightInfo& rFrom, const ActorLightInfo& rTo, f32 rate) {
    MR::blendColor(&pResult->mInfo0.mColor, rFrom.mInfo0.mColor, rTo.mInfo0.mColor, rate);
    MR::blendColor(&pResult->mInfo1.mColor, rFrom.mInfo1.mColor, rTo.mInfo1.mColor, rate);
    MR::blendColor(&pResult->mColor, rFrom.mColor, rTo.mColor, rate);
    ::blendActorLightPos(rFrom.mInfo0, rTo.mInfo0, &pResult->mInfo0, rate);
    ::blendActorLightPos(rFrom.mInfo1, rTo.mInfo1, &pResult->mInfo1, rate);
    pResult->mAlpha2 = MR::getInterpolateValue(rate, rFrom.mAlpha2, rTo.mAlpha2);
}
void LightFunction::calcLightWorldPos(TVec3f* pPos, const LightInfo& rInfo) {
    pPos->x = rInfo.mPos.x;
    pPos->y = rInfo.mPos.y;
    pPos->z = rInfo.mPos.z;

    if (rInfo.mIsFollowCamera) {
        TPos3f mtx;
        mtx.setInline(MR::getCameraViewMtx());
        mtx.invert(mtx);
        mtx.mult(*pPos, *pPos);
    }
}
