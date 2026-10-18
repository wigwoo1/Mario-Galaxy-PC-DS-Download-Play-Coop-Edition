#include "assets.hpp"
#include "frame_controller.hpp"
#include "Game/Screen/WipeKoopa.hpp"
#include "Game/LiveActor/Nerve.hpp"
namespace MR{
bool isFirstStep(const LayoutActor*);void startAnim(LayoutActor*,const char*,u32);J3DFrameCtrl* getAnimCtrl(LayoutActor*,u32);void setAnimRate(LayoutActor*,f32,u32);void showLayout(LayoutActor*);void hideLayout(LayoutActor*);bool isAnimStopped(const LayoutActor*,u32);void setAnimFrameAndStop(LayoutActor*,f32,u32);
}
#undef ATTRIBUTE_WEAK
#define ATTRIBUTE_WEAK
// Original WipeKoopa method bodies and nerves follow; layout backend is native.
namespace {
    static const s32 sAnimFrame = 90;
};  // namespace

namespace {
    NEW_NERVE(WipeKoopaNrvWait, WipeKoopa, Wait);
    NEW_NERVE(WipeKoopaNrvWipeOut, WipeKoopa, WipeOut);
    NEW_NERVE(WipeKoopaNrvClose, WipeKoopa, Close);
};  // namespace

WipeKoopa::WipeKoopa() : WipeLayoutBase("クッパ"), mFrame(::sAnimFrame) {
}

void WipeKoopa::init(const JMapInfoIter& rIter) {
    initNerve(GET_NERVE_ANON(WipeKoopaNrvWait));
    initLayoutManager("WipeKoopa", 1);
}

void WipeKoopa::exeWait() {
}

void WipeKoopa::exeWipeOut() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "out", 0);

        if (mFrame > 0) {
            MR::setAnimRate(this, static_cast< f32 >(MR::getAnimCtrl(this, 0)->getEnd()) / mFrame, 0);
        }

        MR::showLayout(this);
    }

    if (MR::isAnimStopped(this, 0)) {
        setNerve(GET_NERVE_ANON(WipeKoopaNrvClose));
    }
}

void WipeKoopa::exeClose() {
}

void WipeKoopa::wipe(s32 frame) {
    setNerve(GET_NERVE_ANON(WipeKoopaNrvWipeOut));
    MR::hideLayout(this);

    if (frame > 0) {
        mFrame = frame;
    } else {
        mFrame = ::sAnimFrame;
    }
}

void WipeKoopa::forceClose() {
    setNerve(GET_NERVE_ANON(WipeKoopaNrvClose));

    MR::showLayout(this);
    
    MR::startAnim(this, "out", 0);
    MR::setAnimFrameAndStop(this, MR::getAnimCtrl(this, 0)->getEnd(), 0);
}

void WipeKoopa::forceOpen() {
    setNerve(GET_NERVE_ANON(WipeKoopaNrvWait));
    MR::hideLayout(this);
}

bool WipeKoopa::isOpen() const {
    return isNerve(GET_NERVE_ANON(WipeKoopaNrvWait));
}

bool WipeKoopa::isClose() const {
    return isNerve(GET_NERVE_ANON(WipeKoopaNrvClose));
}

bool WipeKoopa::isWipeIn() const {
    return false;
}

bool WipeKoopa::isWipeOut() const {
    return isNerve(GET_NERVE_ANON(WipeKoopaNrvWipeOut));
}

