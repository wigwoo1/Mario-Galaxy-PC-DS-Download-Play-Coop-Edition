#include "Game/Util/GamePadUtil.hpp"
#include "Game/System/WPad.hpp"
#include "Game/System/WPadAcceleration.hpp"
#include "Game/System/WPadButton.hpp"
#include "Game/System/WPadHVSwing.hpp"
#include "Game/System/WPadHolder.hpp"
#include "Game/System/WPadPointer.hpp"
#include "Game/System/WPadStick.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/VectorUtil.hpp"


namespace MR {
    bool isGamePadStickOperated(s32 channel) {
        f32 x = getSubPadStickX(channel);
        f32 y = getSubPadStickY(channel);

        return MR::abs(x) + MR::abs(y) > 0.0f;
    }
    void getCorePadPointingPosBasedOnScreen(TVec2f* pPos, s32 channel) {
        MR::getWPad(channel)->mPointer->getPointingPosBasedOnScreen(pPos);
    }
    void getCorePadPointingPos(TVec2f* pPos, s32 channel) {
        MR::getWPad(channel)->mPointer->getPointingPos(pPos);
    }
    void getCorePadPastPointingPos(TVec2f* pPos, s32 idx, s32 channel) {
        MR::getWPad(channel)->mPointer->getPastPointingPos(pPos, idx);
    }
    s32 getCorePadEnablePastCount(s32 channel) {
        return MR::getWPad(channel)->mPointer->getEnablePastCount();
    }
    bool isCorePadPointInScreen(s32 channel) {
        return MR::getWPad(channel)->mPointer->mIsPointInScreen;
    }
    bool isConnectedWPad(s32 channel) {
        return MR::getWPad(channel)->mIsConnected;
    }
    f32 getCorePadDistanceToDisplay(s32 channel) {
        return MR::getWPad(channel)->mPointer->mDistDisplay;
    }
    bool testSubPadButtonZ(s32 channel) {
        return MR::getWPad(channel)->mButton->testButtonZ();
    }
    bool testCorePadButtonUp(s32 channel) {
        return MR::getWPad(channel)->mButton->testButtonUp();
    }
    bool testCorePadButtonDown(s32 channel) {
        return MR::getWPad(channel)->mButton->testButtonDown();
    }
    bool testCorePadButtonLeft(s32 channel) {
        return MR::getWPad(channel)->mButton->testButtonLeft();
    }
    bool testCorePadButtonRight(s32 channel) {
        return MR::getWPad(channel)->mButton->testButtonRight();
    }
    bool testCorePadButtonA(s32 channel) {
        return MR::getWPad(channel)->mButton->testButtonA();
    }
    bool testCorePadButtonB(s32 channel) {
        return MR::getWPad(channel)->mButton->testButtonB();
    }
    bool testCorePadButton1(s32 channel) {
        return MR::getWPad(channel)->mButton->testButton1();
    }
    bool testCorePadButton2(s32 channel) {
        return MR::getWPad(channel)->mButton->testButton2();
    }
    bool testCorePadButtonPlus(s32 channel) {
        return MR::getWPad(channel)->mButton->testButtonPlus();
    }
    bool testCorePadButtonMinus(s32 channel) {
        return MR::getWPad(channel)->mButton->testButtonMinus();
    }
    bool testSubPadButtonC(s32 channel) {
        return MR::getWPad(channel)->mButton->testButtonC();
    }
    bool testPadButtonAnyWithoutHome(s32 channel) {
        return testCorePadButtonUp(channel) || testCorePadButtonDown(channel) || testCorePadButtonLeft(channel) || testCorePadButtonRight(channel) ||
               testCorePadButtonA(channel) || testCorePadButtonB(channel) || testCorePadButton1(channel) || testCorePadButton2(channel) ||
               testCorePadButtonPlus(channel) || testCorePadButtonMinus(channel) || testSubPadButtonC(channel) || testSubPadButtonZ(channel);
    }
    bool testCorePadTriggerA(s32 channel) {
        return MR::getWPad(channel)->mButton->testTriggerA();
    }
    bool testCorePadTriggerUp(s32 channel) {
        return MR::getWPad(channel)->mButton->testTriggerUp();
    }
    bool testCorePadTriggerDown(s32 channel) {
        return MR::getWPad(channel)->mButton->testTriggerDown();
    }
    bool testCorePadTriggerLeft(s32 channel) {
        return MR::getWPad(channel)->mButton->testTriggerLeft();
    }
    bool testCorePadTriggerRight(s32 channel) {
        return MR::getWPad(channel)->mButton->testTriggerRight();
    }
    bool testCorePadTriggerB(s32 channel) {
        return MR::getWPad(channel)->mButton->testTriggerB();
    }
    bool testCorePadTrigger1(s32 channel) {
        return MR::getWPad(channel)->mButton->testTrigger1();
    }
    bool testCorePadTrigger2(s32 channel) {
        return MR::getWPad(channel)->mButton->testTrigger2();
    }
    bool testCorePadTriggerPlus(s32 channel) {
        return MR::getWPad(channel)->mButton->testTriggerPlus();
    }
    bool testCorePadTriggerMinus(s32 channel) {
        return MR::getWPad(channel)->mButton->testTriggerMinus();
    }
    bool testCorePadTriggerAnyWithoutHome(s32 channel) {
        return testCorePadTriggerUp(channel) || testCorePadTriggerDown(channel) || testCorePadTriggerLeft(channel) ||
               testCorePadTriggerRight(channel) || testCorePadTriggerA(channel) || testCorePadTriggerB(channel) || testCorePadTrigger1(channel) ||
               testCorePadTrigger2(channel) || testCorePadTriggerPlus(channel) || testCorePadTriggerMinus(channel);
    }
}

