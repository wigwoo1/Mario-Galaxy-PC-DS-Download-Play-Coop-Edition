#include "Game/Util/LiveActorUtil.hpp"
// Original actor state timing subset. Interval divisor must be nonzero.
namespace MR {
    bool isStep(const LiveActor* pActor, s32 step) {
        return pActor->getNerveStep() == step;
    }
    bool isFirstStep(const LiveActor* pActor) {
        return isStep(pActor, 0);
    }
    bool isLessStep(const LiveActor* pActor, s32 step) {
        return pActor->getNerveStep() < step;
    }
    bool isLessEqualStep(const LiveActor* pActor, s32 step) {
        return pActor->getNerveStep() <= step;
    }
    bool isGreaterStep(const LiveActor* pActor, s32 step) {
        return pActor->getNerveStep() > step;
    }
    bool isGreaterEqualStep(const LiveActor* pActor, s32 step) {
        return pActor->getNerveStep() >= step;
    }
    bool isIntervalStep(const LiveActor* pActor, s32 step) {
        return pActor->getNerveStep() % step == 0;
    }
    bool isNewNerve(const LiveActor* pActor) {
        return pActor->getNerveStep() < 0;
    }
    void setNerveAtStep(LiveActor* pActor, const Nerve* pNerve, s32 step) {
        if (pActor->getNerveStep() == step) {
            pActor->setNerve(pNerve);
        }
    }
}
