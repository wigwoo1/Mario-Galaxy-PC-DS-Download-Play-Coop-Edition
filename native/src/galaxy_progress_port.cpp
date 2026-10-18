#include "galaxy_progress.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
namespace MR{int clamp(int,int,int);int clamp(int value,int low,long high){return clamp(value,low,int(high));}}
// Native descriptor constructor; original accessor/reset/count method subset follows.
namespace{const char* descriptorName=nullptr;int descriptorStars=0;}
GameDataSomeGalaxyStorage::GameDataSomeGalaxyStorage(const GalaxyStatusAccessor&){assetCheck(descriptorName&&descriptorStars>=0&&descriptorStars<=8,"Missing native galaxy descriptor");mGalaxyName=descriptorName;mPowerStarNum=descriptorStars;resetAllData();}
std::unique_ptr<GameDataSomeGalaxyStorage> createNativeGalaxyProgress(const char* name,int stars){assetCheck(name&&stars>=0&&stars<=8&&!descriptorName,"Invalid native galaxy progress descriptor");descriptorName=name;descriptorStars=stars;try{GalaxyStatusAccessor descriptor;auto result=std::make_unique<GameDataSomeGalaxyStorage>(descriptor);descriptorName=nullptr;return result;}catch(...){descriptorName=nullptr;throw;}}
bool GameDataSomeScenarioAccessor::hasPowerStar() const {
    return mSomeGalaxyStorage->hasPowerStar(mScenarioNum - 1);
}

bool GameDataSomeScenarioAccessor::isAlreadyVisited() const {
    return mSomeGalaxyStorage->isAlreadyVisited(mScenarioNum - 1);
}

s32 GameDataSomeScenarioAccessor::getMaxCoinNum() const {
    return MR::clamp(mSomeGalaxyStorage->getMaxCoinNum(mScenarioNum - 1), 0, 999L);
}

void GameDataSomeScenarioAccessor::setPowerStarFlag(bool val) {
    setBitFlagAccordingToBool(&mSomeGalaxyStorage->mPowerStarOwnedFlags, val);
}

void GameDataSomeScenarioAccessor::setFlagAlreadyVisited(bool val) {
    setBitFlagAccordingToBool(&mSomeGalaxyStorage->mAlreadyVisitedFlags, val);
}

void GameDataSomeScenarioAccessor::updateMaxCoinNum(int coinNum) {
    if (getMaxCoinNum() < coinNum) {
        s32 scenarioNo = mScenarioNum - 1;
        u16 maxCoins = MR::clamp(coinNum, 0, 999L);
        mSomeGalaxyStorage->setMaxCoinNum(scenarioNo, maxCoins);
    }
}

void GameDataSomeScenarioAccessor::setBitFlagAccordingToBool(u8* pFlags, bool val) {
    if (val) {
        *pFlags |= (1 << (mScenarioNum - 1));
    } else {
        *pFlags &= ~(1 << (mScenarioNum - 1));
    }
}

void GameDataSomeGalaxyStorage::resetAllData() {
    mPowerStarOwnedFlags = 0b00000000;
    mAlreadyVisitedFlags = 0b00000000;
    _A = 0;
    _B = 0;

    for (s32 idx = 0; idx < 8; idx++) {
        mMaxCoinNum[idx] = 0;
    }
}

s32 GameDataSomeGalaxyStorage::getPowerStarNumOwned() const {
    s32 numOwned = 0;
    for (s32 scenarioNo = 1; scenarioNo <= mPowerStarNum; scenarioNo++) {
        if (hasPowerStar(scenarioNo - 1)) {
            numOwned++;
        }
    }
    return numOwned;
}




