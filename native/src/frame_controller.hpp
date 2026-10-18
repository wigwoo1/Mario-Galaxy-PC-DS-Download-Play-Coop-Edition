#pragma once
#include <revolution/types.h>
// Petari J3DFrameCtrl subset; original bodies with inline linkage for this native header.
class J3DFrameCtrl {
public:
    enum Attribute_e {
        /*  -1 */ EMode_NULL = -1,
        /* 0x0 */ EMode_NONE,
        /* 0x1 */ EMode_RESET,
        /* 0x2 */ EMode_LOOP,
        /* 0x3 */ EMode_REVERSE,
        /* 0x4 */ EMode_LOOP_REVERSE,
    };

    inline J3DFrameCtrl(s16 endFrame) {
        init(endFrame);
    }

    J3DFrameCtrl() {
        this->init(0);
    }
    void init(s16);
    void init(int endFrame) {
        init((s16)endFrame);
    }
    BOOL checkPass(f32);
    void update();
    virtual ~J3DFrameCtrl() {
    }

    u8 getAttribute() const {
        return mAttribute;
    }
    void setAttribute(u8 attr) {
        mAttribute = attr;
    }
    u8 getState() const {
        return mState;
    }
    u8 andState(u8 state) const {
        return mState & state;
    }
    bool checkState(u8 state) const {
        return mState & state ? true : false;
    }
    s16 getStart() const {
        return mStart;
    }
    void setStart(s16 start) {
        mStart = start;
        mFrame = start;
    }
    s16 getEnd() const {
        return mEnd;
    }
    void setEnd(s16 end) {
        mEnd = end;
    }
    s16 getLoop() const {
        return mLoop;
    }
    void setLoop(s16 loop) {
        mLoop = loop;
    }
    f32 getRate() const {
        return mRate;
    }
    void setRate(f32 rate) {
        mRate = rate;
    }
    f32 getFrame() const {
        return mFrame;
    }
    void setFrame(f32 frame) {
        mFrame = frame;
    }
    void reset() {
        mFrame = mStart;
        mRate = 1.0f;
        mState = 0;
    }

    /* 0x04 */ u8 mAttribute;
    /* 0x05 */ u8 mState;
    /* 0x06 */ s16 mStart;
    /* 0x08 */ s16 mEnd;
    /* 0x0A */ s16 mLoop;
    /* 0x0C */ f32 mRate;
    /* 0x10 */ f32 mFrame;
};  // Size: 0x1
inline void J3DFrameCtrl::init(s16 endFrame) {
    mAttribute = EMode_LOOP;
    mState = 0;
    mStart = 0;
    mEnd = endFrame;
    mLoop = 0;
    mRate = 1.0f;
    mFrame = 0.0f;
}

inline int J3DFrameCtrl::checkPass(f32 passFrame) {
    f32 next_frame = mFrame + mRate;

    switch (mAttribute) {
    case 0:
    case 1:
        if (next_frame < mStart) {
            next_frame = mStart;
        }

        if (next_frame >= mEnd) {
            next_frame = mEnd - 0.001f;
        }

        if (mFrame <= next_frame) {
            if (mFrame <= passFrame && passFrame < next_frame) {
                return true;
            } else {
                return false;
            }
        }

        if (next_frame <= passFrame && passFrame < mFrame) {
            return true;
        }

        return false;
    case 2:
        if (mFrame < mStart) {
            while (next_frame < mStart) {
                if (mLoop - mStart <= 0.0f) {
                    break;
                }

                next_frame += mLoop - mStart;
            }

            if (next_frame <= passFrame && passFrame < mLoop) {
                return true;
            } else {
                return false;
            }
        } else if (mEnd <= mFrame) {
            while (next_frame >= mEnd) {
                if (mEnd - mLoop <= 0.0f) {
                    break;
                }

                next_frame -= mEnd - mLoop;
            }

            if (mLoop <= passFrame && passFrame < next_frame) {
                return true;
            } else {
                return false;
            }
        } else if (next_frame < mStart) {
            while (next_frame < mStart) {
                if (mLoop - mStart <= 0.0f) {
                    break;
                }

                next_frame += mLoop - mStart;
            }

            if ((mStart <= passFrame && passFrame < mFrame) || (next_frame <= passFrame && passFrame < mLoop)) {
                return true;
            } else {
                return false;
            }
        } else if (mEnd <= next_frame) {
            while (next_frame >= mEnd) {
                if (mEnd - mLoop <= 0.0f) {
                    break;
                }

                next_frame -= mEnd - mLoop;
            }

            if ((mFrame <= passFrame && passFrame < mEnd) || (mLoop <= passFrame && passFrame < next_frame)) {
                return true;
            } else {
                return false;
            }
        } else if (mFrame <= next_frame) {
            if (mFrame <= passFrame && passFrame < next_frame) {
                return true;
            } else {
                return false;
            }
        } else if (next_frame <= passFrame && passFrame < mFrame) {
            return true;
        }

        return false;
    case 3:
    case 4:
        if (next_frame >= mEnd) {
            next_frame = mEnd - 0.001f;
        }

        if (next_frame < mStart) {
            next_frame = mStart;
        }

        if (mFrame <= next_frame) {
            if (mFrame <= passFrame && passFrame < next_frame) {
                return true;
            } else {
                return false;
            }
        }

        if (next_frame <= passFrame && passFrame < mFrame) {
            return true;
        }

        return false;
    default:
        return false;
    }
}

inline void J3DFrameCtrl::update() {
    mState = 0;
    mFrame += mRate;

    switch (mAttribute) {
    case EMode_NONE:
        if (mFrame < mStart) {
            mFrame = mStart;
            mRate = 0.0f;
            mState |= (u8)1;
        }

        if (mFrame >= mEnd) {
            mFrame = mEnd - 0.001f;
            mRate = 0.0f;
            mState |= (u8)1;
        }

        break;
    case EMode_RESET:
        if (mFrame < mStart) {
            mFrame = mStart;
            mRate = 0.0f;
            mState |= (u8)1;
        }

        if (mFrame >= mEnd) {
            mFrame = mStart;
            mRate = 0.0f;
            mState |= (u8)1;
        }

        break;
    case EMode_LOOP:
        while (mFrame < mStart) {
            mState |= (u8)2;
            if (mLoop - mStart <= 0.0f) {
                break;
            }

            mFrame += mLoop - mStart;
        }

        while (mFrame >= mEnd) {
            mState |= (u8)2;
            if (mEnd - mLoop <= 0.0f) {
                break;
            }

            mFrame -= mEnd - mLoop;
        }

        break;
    case EMode_REVERSE:
        if (mFrame >= mEnd) {
            mFrame = mEnd - (mFrame - mEnd);
            mRate = -mRate;
        }

        if (mFrame < mStart) {
            mFrame = mStart - (mFrame - mStart);
            mRate = 0.0f;
            mState |= (u8)1;
        }

        break;
    case EMode_LOOP_REVERSE:
        if (mFrame >= mEnd - 1.0f) {
            mFrame = (mEnd - 1.0f) - (mFrame - (mEnd - 1.0f));
            mRate = -mRate;
        }

        if (mFrame < mStart) {
            mFrame = mStart - (mFrame - mStart);
            mRate = -mRate;
            mState |= (u8)2;
        }

        break;
    }
}


