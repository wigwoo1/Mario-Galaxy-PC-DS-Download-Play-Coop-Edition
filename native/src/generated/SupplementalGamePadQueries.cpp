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
    f32 getSubPadStickX(s32 channel) {
        return MR::getWPad(channel)->mStick->mStick.x;
    }
    f32 getSubPadStickY(s32 channel) {
        return MR::getWPad(channel)->mStick->mStick.y;
    }
    bool testSubPadTriggerZ(s32 channel) {
        return MR::getWPad(channel)->mButton->testTriggerZ();
    }
    bool testSubPadTriggerC(s32 channel) {
        return MR::getWPad(channel)->mButton->testTriggerC();
    }
    void getCorePadAcceleration(TVec3f* pAccel, s32 channel) {
        MR::getWPad(channel)->mCorePadAccel->getAcceleration(pAccel);
    }
    void getSubPadAcceleration(TVec3f* pAccel, s32 channel) {
        MR::getWPad(channel)->mSubPadAccel->getAcceleration(pAccel);
    }
    bool isCorePadSwing(s32 channel) {
        return MR::getWPad(channel)->mCorePadSwing->mIsSwing;
    }
    bool isSubPadSwing(s32 channel) {
        return MR::getWPad(channel)->mSubPadSwing->mIsSwing;
    }
    bool isPadSwing(s32 channel) {
        return isCorePadSwing(channel) || isSubPadSwing(channel);
    }
    void calcWorldStickDirectionXZ(f32* pDirX, f32* pDirZ, s32 channel) {
        TPos3f mtx;
        mtx.set(getCameraInvViewMtx());
        TVec3f side;
        mtx.getXDir(side);
        side.y = 0.0f;
        normalizeOrZero(&side);
        TVec3f front;
        mtx.getZDir(front);
        front.y = 0.0f;
        normalizeOrZero(&front);
        front.scale(-1.0f);

        f32 x = getSubPadStickX(channel);
        f32 y = getSubPadStickY(channel);
        side.scale(x);
        front.scale(y);
        TVec3f direction = side + front;
        normalizeOrZero(&direction);
        *pDirX = direction.x;
        *pDirZ = direction.z;
    }
    void calcWorldStickDirectionXZ(TVec3f* pDir, s32 channel) {
        pDir->y = 0.0f;

        calcWorldStickDirectionXZ(&pDir->x, &pDir->z, channel);
    }
    u32 getWPadMaxCount() {
        return 2;
    }

    bool testFpViewStartTrigger() {
        return testCorePadTriggerUp(WPAD_CHAN0);
    }
    bool testDPDMenuPadDecideTrigger() {
        return testCorePadTriggerA(WPAD_CHAN0);
    }
    bool testSystemPadTriggerDecide() {
        return testCorePadTriggerA(WPAD_CHAN0) != false;
    }
    bool isOperatingWPad(s32 channel) {
        WPad* pWPad = MR::getWPad(channel);

        if (!pWPad->mCorePadAccel->isBalanced()) {
            return true;
        }

        if (pWPad->mButton->testButtonA()) {
            return true;
        }

        return pWPad->mButton->testButtonB();
    }
    bool testCorePadTriggerHome(s32 channel) {
        return MR::getWPad(channel)->mButton->testTriggerHome();
    }
    bool getPlayerTriggerB() {
        return testCorePadTriggerB(WPAD_CHAN0);
    }
    bool getPlayerTriggerA() {
        return testCorePadTriggerA(WPAD_CHAN0);
    }
    bool getPlayerTriggerZ() {
        return testSubPadTriggerZ(WPAD_CHAN0);
    }
    bool getPlayerTriggerC() {
        return testSubPadTriggerC(WPAD_CHAN0);
    }
    bool isCorePadSwingTrigger(s32 channel) {
        return MR::getWPad(channel)->mCorePadSwing->mIsTriggerSwing;
    }
    bool testFpViewOutTrigger() {
        return testCorePadTriggerDown(WPAD_CHAN0) || testCorePadTriggerA(WPAD_CHAN0);
    }
    bool testSystemTriggerA() {
        return testCorePadTriggerA(WPAD_CHAN0) != false;
    }
    bool testSystemTriggerB() {
        return testCorePadTriggerB(WPAD_CHAN0) != false;
    }
    bool testSubPadStickTriggerDown(s32 channel) {
        return (MR::getWPad(channel)->mStick->mTrigger & 2) != 0;
    }
    bool testSubPadStickTriggerUp(s32 channel) {
        return (MR::getWPad(channel)->mStick->mTrigger & 1) != 0;
    }
}
