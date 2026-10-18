#include "Game/Util/ActorStateUtil.hpp"
#include "Game/LiveActor/ActorStateBase.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/LiveActor/ActorStateKeeper.hpp"
#include "Game/LiveActor/Spine.hpp"
#ifndef NO_INLINE
#define NO_INLINE __attribute__((noinline))
#endif
#include "Game/Util/NerveUtil.hpp"
// Original update helper subsets from pinned ActorStateUtil.cpp and NerveUtil.cpp.
namespace MR {
    bool updateActorState(LiveActor* pActor, ActorStateBaseInterface* pActorState) {
        if (isFirstStep(pActor)) {
            pActorState->appear();
        }

        if (pActorState->update()) {
            if (!pActorState->mIsDead) {
                pActorState->kill();
            }

            return true;
        }

        return false;
    }

    bool updateActorStateAndNextNerve(LiveActor* pActor, ActorStateBaseInterface* pActorState, const Nerve* pNextNerve) {
        bool isStateUpdated = updateActorState(pActor, pActorState);

        if (isStateUpdated) {
            pActor->setNerve(pNextNerve);
            return true;
        }

        return false;
    }

}

namespace MR {
bool isStep(const NerveExecutor* pExecutor,s32 step){return pExecutor->getNerveStep()==step;}
bool isFirstStep(const NerveExecutor* pExecutor){return isStep(pExecutor,0);}
    bool updateActorState(NerveExecutor* pExecutor) {
        return pExecutor->mSpine->mStateKeeper->updateCurrentState();
    }

    bool updateActorState(NerveExecutor* pExecutor, ActorStateBaseInterface* pActorState) {
        if (isFirstStep(pExecutor)) {
            pActorState->appear();
        }

        if (pActorState->update()) {
            if (!pActorState->mIsDead) {
                pActorState->kill();
            }

            return true;
        }

        return false;
    }

    bool updateActorStateAndNextNerve(NerveExecutor* pExecutor, const Nerve* pNextNerve) {
        bool isStateUpdated = updateActorState(pExecutor);

        if (isStateUpdated) {
            pExecutor->setNerve(pNextNerve);
            return true;
        }

        return false;
    }

    bool updateActorStateAndNextNerve(NerveExecutor* pExecutor, ActorStateBaseInterface* pActorState, const Nerve* pNextNerve) {
        bool isStateUpdated = updateActorState(pExecutor, pActorState);

        if (isStateUpdated) {
            pExecutor->setNerve(pNextNerve);
            return true;
        }

        return false;
    }
}

