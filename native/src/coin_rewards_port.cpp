#include "coin_rewards.hpp"
#include <algorithm>
#define NO_INLINE __attribute__((noinline))
#define COIN_NUM_MIN 0
#define COIN_NUM_MAX 999
#define COIN_STEP_NUM 50
#define PURPLE_COIN_POWER_STAR_APPEAR_NUM 100
namespace{NativeCoinRewardEvents* activeEvents=nullptr;NativeCoinRewardEvents& events(){if(!activeEvents)throw std::logic_error("Unbound native coin reward events");return *activeEvents;}}
void bindNativeCoinRewardEvents(NativeCoinRewardEvents* value){if(!value||activeEvents)throw std::logic_error("Invalid/nested coin reward event binding");activeEvents=value;}
void unbindNativeCoinRewardEvents(){activeEvents=nullptr;}
namespace MR{int clamp(int value,int low,int high){return std::clamp(value,low,high);}void incPlayerLeft(){auto& reward=events();if(reward.deliverLife)reward.deliverLife();++reward.lifeIncrements;}void requestOneUp(){++events().oneUpRequests;}void requestAppearPowerStarCoin100(){++events().purpleStarRequests;}}
// Original coin/purple methods, constructor/destructor subset; Star Bits pending.
namespace {
    void requestOneUpIfNeed(int prevNum, int currNum, int stepNum) NO_INLINE {
        if (prevNum / stepNum < currNum / stepNum) {
            MR::incPlayerLeft();
            MR::requestOneUp();
        }
    }
};  // namespace

void ScenePlayingResult::incCoin(int num) {
    s32 prevNum = mCoinNum;
    s32 currNum = MR::clamp(mCoinNum + num, COIN_NUM_MIN, COIN_NUM_MAX);
    mCoinNum = currNum;

    ::requestOneUpIfNeed(prevNum, currNum, COIN_STEP_NUM);
}

s32 ScenePlayingResult::getCoinNum() const {
    return MR::clamp(mCoinNum, COIN_NUM_MIN, COIN_NUM_MAX);
}

void ScenePlayingResult::incPurpleCoin() {
    mPurpleCoinNum++;

    if (mPurpleCoinNum == PURPLE_COIN_POWER_STAR_APPEAR_NUM) {
        MR::requestAppearPowerStarCoin100();
    }
}

ScenePlayingResult::~ScenePlayingResult() {
}

ScenePlayingResult::ScenePlayingResult() : NameObj("プレイ結果保持"), mCoinNum(0), mPurpleCoinNum(0) {
}


