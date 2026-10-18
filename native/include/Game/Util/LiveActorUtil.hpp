#pragma once
#include "Game/LiveActor/LiveActor.hpp"
namespace MR {inline bool isDead(const LiveActor* host){return host->isDead();}}

namespace MR {
bool isStep(const LiveActor* pActor, s32 step);
bool isFirstStep(const LiveActor* pActor);
bool isLessStep(const LiveActor* pActor, s32 step);
bool isLessEqualStep(const LiveActor* pActor, s32 step);
bool isGreaterStep(const LiveActor* pActor, s32 step);
bool isGreaterEqualStep(const LiveActor* pActor, s32 step);
bool isIntervalStep(const LiveActor* pActor, s32 step);
bool isNewNerve(const LiveActor* pActor);
void setNerveAtStep(LiveActor* pActor, const Nerve* pNerve, s32 step);
}

