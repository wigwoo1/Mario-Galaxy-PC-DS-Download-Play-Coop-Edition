#include "bck_settings.hpp"
// Original BckCtrl setting application bodies, unchanged.
inline f32 calcPlayRate(const J3DFrameCtrl& rCtrl, s32 playFrame, f32 baseRate = 1.0f) {
    return baseRate * (static_cast< f32 >(rCtrl.getEnd() - rCtrl.getStart()) / playFrame);
}

void BckCtrlFunction::reflectBckCtrlData(const BckCtrlData& rData, XanimePlayer* pPlayer) {
    XanimeFrameCtrl* pFrameCtrl = pPlayer->getFrameCtrl();

    if (rData.mStartFrame >= 0 && rData.mStartFrame <= pFrameCtrl->getEnd()) {
        pFrameCtrl->setStart(rData.mStartFrame);
        pFrameCtrl->setLoop(rData.mStartFrame);
        pPlayer->_84 = pPlayer->_20->getFrame();
    }

    if (rData.mEndFrame >= 0 && rData.mEndFrame <= pFrameCtrl->getEnd()) {
        pFrameCtrl->setEnd(rData.mEndFrame);
    }

    if (rData.mRepeatFrame >= 0 && rData.mRepeatFrame <= pFrameCtrl->getEnd()) {
        pFrameCtrl->setLoop(rData.mRepeatFrame);
    }

    if (rData.mPlayFrame >= 0) {
        f32 speed = 0.0f;
        if (rData.mPlayFrame != 0) {
            speed = calcPlayRate(*pFrameCtrl, rData.mPlayFrame);
        }

        pPlayer->changeSpeed(speed);
    }

    if (rData.mInterpole >= 0) {
        pPlayer->changeInterpoleFrame(rData.mInterpole);
    }

    if (rData.mLoopMode != 0xFF) {
        pFrameCtrl->setAttribute(rData.mLoopMode);
    }
}

