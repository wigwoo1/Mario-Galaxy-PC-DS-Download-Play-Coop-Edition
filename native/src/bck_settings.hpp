#pragma once
#include "frame_controller.hpp"
// Original BckCtrlData; native XanimePlayer setting adapter only, not full player.
class BckCtrlData {
public:
    BckCtrlData() : mName(""), mPlayFrame(-1), mStartFrame(-1), mEndFrame(-1), mRepeatFrame(-1), mInterpole(-1), mLoopMode(-1) {
    }

    /* 0x00 */ const char* mName;
    /* 0x04 */ s16 mPlayFrame;
    /* 0x06 */ s16 mStartFrame;
    /* 0x08 */ s16 mEndFrame;
    /* 0x0A */ s16 mRepeatFrame;
    /* 0x0C */ s16 mInterpole;
    /* 0x0E */ u8 mLoopMode;
    /* 0x0F */ u8 _F;
    /* 0x10 */ u8 _10;
    /* 0x11 */ u8 _11;
};
class XanimeFrameCtrl:public J3DFrameCtrl {};
class XanimePlayer {public:XanimeFrameCtrl controller;XanimeFrameCtrl* _20=&controller;float _84=0;int interpolation=-1;XanimeFrameCtrl* getFrameCtrl(){return &controller;}void changeSpeed(float speed){controller.setRate(speed);}void changeInterpoleFrame(int frames){interpolation=frames;}};
class BckCtrlFunction {public:static void reflectBckCtrlData(const BckCtrlData&,XanimePlayer*);};
