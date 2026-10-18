#include <Native/ScenarioBinding.hpp>
namespace { const ScenarioData* getCurrentScenarioData() { if (!nativeScenarioBinding) throw std::runtime_error("Native scenario services are not bound"); return ScenarioDataFunction::getScenarioDataParser()->getScenarioData(nativeScenarioBinding->galaxyName.c_str()); } }
namespace ScenarioDataFunction {
    ScenarioDataParser* getScenarioDataParser() NO_INLINE {
        if (nativeScenarioBinding) return nativeScenarioBinding->parser;
#if defined(GALAXY_NATIVE_SCENARIO_SOURCE_ONLY)
        throw std::runtime_error("Native scenario services are not bound");
#else
        return SingletonHolder< GameSystem >::get()->mSceneController->mScenarioParser;
#endif
    }

    u32 getCurrentCommonLayers(const char* pParam1) {
        ::getCurrentScenarioData();

        return 1;
    }

    u32 getCurrentScenarioLayers(const char* pParam1, s32 param2) {
        u32 layerNum;

        if (::getCurrentScenarioData()->getValueU32(pParam1, param2, &layerNum)) {
            return layerNum * 2;
        }

        return 0;
    }
};  // namespace ScenarioDataFunction

