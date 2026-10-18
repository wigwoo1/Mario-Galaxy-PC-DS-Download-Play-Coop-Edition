#include <Game/System/ScenarioDataParser.hpp>
#include <Game/Util/StringUtil.hpp>
s32 ScenarioData::getScenarioNum() const {
    s32 count = 0;
    for (s32 scenarioNo = 1; scenarioNo <= mScenarioData->getNumEntries(); scenarioNo++) {
        bool hidden = false;
        getValueBool("IsHidden", scenarioNo, &hidden);
        if (!hidden) {
            count++;
        }
    }

    return count;
}

s32 ScenarioData::getPowerStarNum() const {
    s32 count = 0;
    for (s32 scenarioNo = 1; scenarioNo <= mScenarioData->getNumEntries(); scenarioNo++) {
        u32 powerStarId = 0;
        getValueU32("PowerStarId", scenarioNo, &powerStarId);
        if (powerStarId != 0) {
            count++;
        }
    }

    return count;
}

bool ScenarioData::getValueString(const char* pKey, s32 scenarioNo, const char** ppOut) const {
    JMapInfoIter iter = getScenarioDataIter(scenarioNo);
    bool isExist = iter.getValue(pKey, ppOut);

    if (isExist) {
        if (MR::isEqualString(*ppOut, "")) {
            *ppOut = nullptr;
        }
    }

    return isExist;
}

const char* ScenarioData::getZoneName(s32 zoneId) const {
    const char* pZoneName = nullptr;
    mZoneList->getValue< const char* >(zoneId, "ZoneName", &pZoneName);

    return pZoneName;
}

JMapInfoIter ScenarioData::getScenarioDataIter(s32 scenarioNo) const {
    JMapInfoIter iter = mScenarioData->findElement< s32 >("ScenarioNo", scenarioNo, 0);
    return iter;
}

bool ScenarioData::getValueU32(const char* pKey, s32 scenarioNo, u32* pOut) const {
    JMapInfoIter iter = getScenarioDataIter(scenarioNo);
    return iter.getValue(pKey, pOut);
}

bool ScenarioData::getValueBool(const char* pKey, s32 scenarioNo, bool* pOut) const {
    JMapInfoIter iter = getScenarioDataIter(scenarioNo);
    return iter.getValue(pKey, pOut);
}

s32 ScenarioData::getZoneNum() const {
    if (mZoneList->mData != nullptr) {
        return mZoneList->mData->mNumEntries;
    }

    return 0;
}

s32 ScenarioData::getZoneId(const char* pZoneName) const {
    for (s32 i = 0; i < getZoneNum(); i++) {
        const char* pName = nullptr;
        mZoneList->getValue< const char* >(i, "ZoneName", &pName);

        if (MR::isEqualStringCase(pName, pZoneName)) {
            return i;
        }
    }

    return 0;
}

