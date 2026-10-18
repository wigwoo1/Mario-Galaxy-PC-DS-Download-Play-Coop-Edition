#include <revolution/gx/GXLighting.h>
#include <Game/Map/LightFunction.hpp>
#include <Game/Map/LightDirector.hpp>
#include <Game/AreaObj/LightAreaHolder.hpp>
#include <Game/Scene/SceneObjHolder.hpp>
#include <Game/Util/ObjUtil.hpp>
#include <Game/Util/JMapInfo.hpp>
#include <cstdio>
void LightFunction::initLightData() {
    MR::getSceneObj< LightDirector >(SceneObj_LightDirector)->initData();
}
ResourceHolder* LightFunction::loadLightArchive() {
    char buf[0x100];
    snprintf(buf, sizeof(buf), "LightData.arc");
    return MR::createAndAddResourceHolder(buf);
}
s32 LightFunction::createLightDataParser(JMapInfo** pOut) {
    ResourceHolder* holder = MR::getSceneObj< LightDirector >(SceneObj_LightDirector)->mResourceHolder;
    JMapInfo* info = MR::tryCreateCsvParser(holder, "LightData.bcsv");
    *pOut = info;
    if (info->mData != nullptr) {
        return info->mData->mNumEntries;
    }

    return 0;
}
s32 LightFunction::createZoneDataParser(const char* pZone, JMapInfo** pOut) {
    ResourceHolder* holder = MR::getSceneObj< LightDirector >(SceneObj_LightDirector)->mResourceHolder;
    JMapInfo* info = MR::tryCreateCsvParser(holder, "Light%s.bcsv", pZone);
    *pOut = info;

    if (info != nullptr) {
        if (info->mData != nullptr) {
            return info->mData->mNumEntries;
        }

        return 0;
    }

    return 0;
}
const char* LightFunction::getDefaultAreaLightName() {
    return MR::getSceneObj< LightDirector >(SceneObj_LightDirector)->mDataHolder->getDefaultAreaLightName();
}
s32 LightFunction::getDefaultStepInterpolate() {
    return MR::getSceneObj< LightDirector >(SceneObj_LightDirector)->mDataHolder->getDefaultStepInterpolate();
}
void LightFunction::registerPlayerLightCtrl(const ActorLightCtrl* pCtrl) {
    MR::getSceneObj< LightDirector >(SceneObj_LightDirector)->_1C = pCtrl;
}
void LightFunction::registerLightAreaHolder(LightAreaHolder* pHolder) {
    MR::getSceneObj< LightDirector >(SceneObj_LightDirector)->_C = pHolder;
}
bool LightFunction::tryFindNewAreaLightID(const TVec3f& rPos, ZoneLightID* pId) {
    return MR::getSceneObj< LightDirector >(SceneObj_LightDirector)->_C->tryFindLightID(rPos, pId);
}
AreaLightInfo* LightFunction::getAreaLightInfo(const ZoneLightID& rId) {
    const char* name = MR::getSceneObj< LightDirector >(SceneObj_LightDirector)->mZoneDataHolder->getAreaLightNameInZoneData(rId);
    return MR::getSceneObj< LightDirector >(SceneObj_LightDirector)->mDataHolder->findAreaLight(name);
}
