#include <Game/System/ScenarioDataParser.hpp>
#include <Game/System/GalaxyNameSortTable.hpp>
#include <Game/Util/StringUtil.hpp>
#include <algorithm>
#include <stdexcept>
namespace {
    inline const char* getGalaxyName(const ScenarioData* pData) {
        return pData->mGalaxyName;
    }

    struct GalaxyNameSortLt {
        bool operator()(ScenarioData* pLhs, ScenarioData* pRhs) {
            return static_cast< u32 >(GalaxyNameSortTable::getGalaxySortIndex(getGalaxyName(pLhs))) <
                   static_cast< u32 >(GalaxyNameSortTable::getGalaxySortIndex(getGalaxyName(pRhs)));
        }
    };

}
ScenarioDataParser::ScenarioDataParser(const char* pName, NativeScenarioSource* nativeSource) : NameObj(pName), mScenarioData() { if (!nativeSource) nativeSource = nativeScenarioStartupSource; if (nativeSource) { auto source = nativeSource->scenarioList(); if (source.size() > 64) throw std::runtime_error("Native scenario list exceeds original capacity"); std::vector<std::unique_ptr<ScenarioData>> owned; for (const ScenarioData* data : source) { if (!data) throw std::runtime_error("Null native scenario source entry"); owned.emplace_back(new ScenarioData(*data)); } for (auto& data : owned) mScenarioData.mArray[mScenarioData.mCount++] = data.release(); std::sort(mScenarioData.begin(), mScenarioData.end(), GalaxyNameSortLt()); return; }
#if defined(GALAXY_NATIVE_SCENARIO_SOURCE_ONLY)
    throw std::runtime_error("Native scenario source required");
#else
    DVDDir dir;
    DVDDirEntry dirent;
    DVDOpenDir("/StageData", &dir);

    while (DVDReadDir(&dir, &dirent)) {
        if (!dirent.isDir) {
            continue;
        }

        char fileName[256];
        MR::makeScenarioArchiveFileName(fileName, sizeof(fileName), dirent.name);

        if (!MR::isFileExist(fileName, false)) {
            continue;
        }

        ScenarioData* pData = new ScenarioData(fileName);

        u32 index = mScenarioData.mCount;
        u32 nextCount = index;
        nextCount++;

        mScenarioData.mArray[index] = pData;
        mScenarioData.mCount = nextCount;
    }

    DVDCloseDir(&dir);

    std::sort(mScenarioData.begin(), mScenarioData.end(), GalaxyNameSortLt());
#endif
}
const ScenarioData* ScenarioDataParser::getScenarioData(const char* pGalaxyName) const {
    for (s32 i = 0; i < mScenarioData.size(); i++) {
        const ScenarioData* pScenarioData = mScenarioData[i];

        if (MR::isEqualStringCase(pScenarioData->mGalaxyName, pGalaxyName)) {
            return pScenarioData;
        }
    }

    return nullptr;
}

const ScenarioData* ScenarioDataParser::getScenarioData(s32 index) const {
    return mScenarioData[index];
}


ScenarioDataParser::ScenarioDataParser(const char* pName) : ScenarioDataParser(pName, nullptr) {}
