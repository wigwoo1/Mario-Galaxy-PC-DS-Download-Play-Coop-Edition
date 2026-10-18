#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Spine.hpp"
// Original actor nerve access/init methods only; native wrapper owns the spine.
void LiveActor::setNerve(const Nerve* pNerve) {
    mSpine->setNerve(pNerve);
}
bool LiveActor::isNerve(const Nerve* pNerve) const {
    return mSpine->getCurrentNerve() == pNerve;
}
s32 LiveActor::getNerveStep() const {
    return mSpine->mStep;
}
void LiveActor::initNerve(const Nerve* pNerve) {
    mSpine = new Spine(this, pNerve);
}
