#include <revolution/gx/GXLighting.h>
#include <Game/Map/LightFunction.hpp>
#include <Game/Util/ObjUtil.hpp>
#include <Game/Util/JMapInfo.hpp>
#include <cstdio>
namespace {
    void getDataLightInfo(JMapInfo* pInfo, int id, LightInfo* pLightInfo, const char* pName) {
        char colorBuf[256];
        snprintf(colorBuf, sizeof(colorBuf), "%sColor", pName);
        MR::getCsvDataColor(&pLightInfo->mColor, pInfo, colorBuf, id);
        char posBuf[256];
        snprintf(posBuf, sizeof(posBuf), "%sPos", pName);
        MR::getCsvDataVec(&pLightInfo->mPos, pInfo, posBuf, id);
        char camBuf[128];
        snprintf(camBuf, sizeof(camBuf), "%sFollowCamera", pName);
        MR::getCsvDataBool(&pLightInfo->mIsFollowCamera, pInfo, camBuf, id);
    }
    void getDataActorLightInfo(JMapInfo* pInfo, int lightID, ActorLightInfo* pActorInfo, const char* pName) {
        char lightBuf[256];
        snprintf(lightBuf, sizeof(lightBuf), "%sLight%d", pName, 0);
        getDataLightInfo(pInfo, lightID, &pActorInfo->mInfo0, lightBuf);
        snprintf(lightBuf, sizeof(lightBuf), "%sLight%d", pName, 1);
        getDataLightInfo(pInfo, lightID, &pActorInfo->mInfo1, lightBuf);
        char alphaBuf[256];
        snprintf(alphaBuf, sizeof(alphaBuf), "%sAlpha2", pName);
        MR::getCsvDataU8(&pActorInfo->mAlpha2, pInfo, alphaBuf, lightID);
        char ambBuf[256];
        snprintf(ambBuf, sizeof(ambBuf), "%sAmbient", pName);
        MR::getCsvDataColor(&pActorInfo->mColor, pInfo, ambBuf, lightID);
    }
}
void LightFunction::getAreaLightLightData(JMapInfo* pInfo, int idx, AreaLightInfo* pLightInfo) {
    MR::getCsvDataStr(&pLightInfo->mAreaLightName, pInfo, "AreaLightName", idx);
    MR::getCsvDataS32(&pLightInfo->mInterpolate, pInfo, "Interpolate", idx);
    ::getDataActorLightInfo(pInfo, idx, &pLightInfo->mPlayerLight, "Player");
    ::getDataActorLightInfo(pInfo, idx, &pLightInfo->mStrongLight, "Strong");
    ::getDataActorLightInfo(pInfo, idx, &pLightInfo->mWeakLight, "Weak");
    ::getDataActorLightInfo(pInfo, idx, &pLightInfo->mPlanetLight, "Planet");
    MR::getCsvDataBool(&pLightInfo->mFix, pInfo, "Fix", idx);
}
