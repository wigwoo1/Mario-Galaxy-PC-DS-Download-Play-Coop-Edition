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


