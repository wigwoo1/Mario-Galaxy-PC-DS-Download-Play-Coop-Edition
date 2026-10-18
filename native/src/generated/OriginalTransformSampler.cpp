#include "OriginalKeySampler.hpp"
J3DAnmTransform::J3DAnmTransform(s16 frameMax, f32* pScaleData, s16* pRotData, f32* pTransData) : J3DAnmBase(frameMax) {
    mScaleData = pScaleData;
    mRotData = pRotData;
    mTransData = pTransData;
    field_0x18 = 0;
    field_0x1a = 0;
    field_0x1c = 0;
    field_0x1e = 0;
}
void J3DAnmTransformKey::calcTransform(f32 frame, u16 jointNo, J3DTransformInfo* pTransform) const {
    u16 idx = jointNo * 3;
    J3DAnmTransformKeyTable* entryX = &mAnmTable[idx];
    J3DAnmTransformKeyTable* entryY = &mAnmTable[idx + 1];
    J3DAnmTransformKeyTable* entryZ = &mAnmTable[idx + 2];

    switch (entryX->mScaleInfo.mMaxFrame) {
    case 0:
        pTransform->mScale.x = 1.0f;
        break;
    case 1:
        pTransform->mScale.x = mScaleData[entryX->mScaleInfo.mOffset];
        break;
    default:
        pTransform->mScale.x = J3DGetKeyFrameInterpolation(frame, &entryX->mScaleInfo, &mScaleData[entryX->mScaleInfo.mOffset]);
    }

    switch (entryY->mScaleInfo.mMaxFrame) {
    case 0:
        pTransform->mScale.y = 1.0f;
        break;
    case 1:
        pTransform->mScale.y = mScaleData[entryY->mScaleInfo.mOffset];
        break;
    default:
        pTransform->mScale.y = J3DGetKeyFrameInterpolation(frame, &entryY->mScaleInfo, &mScaleData[entryY->mScaleInfo.mOffset]);
    }

    switch (entryZ->mScaleInfo.mMaxFrame) {
    case 0:
        pTransform->mScale.z = 1.0f;
        break;
    case 1:
        pTransform->mScale.z = mScaleData[entryZ->mScaleInfo.mOffset];
        break;
    default:
        pTransform->mScale.z = J3DGetKeyFrameInterpolation(frame, &entryZ->mScaleInfo, &mScaleData[entryZ->mScaleInfo.mOffset]);
    }

    switch (entryX->mRotationInfo.mMaxFrame) {
    case 0:
        pTransform->mRotation.x = 0;
        break;
    case 1:
        pTransform->mRotation.x = static_cast<s16>(u16(u32(static_cast<s32>(mRotData[entryX->mRotationInfo.mOffset])) << mDecShift));
        break;
    default:
        pTransform->mRotation.x = static_cast<s16>(u16(u32(static_cast<s32>((int)J3DGetKeyFrameInterpolation(frame, &entryX->mRotationInfo, &mRotData[entryX->mRotationInfo.mOffset]))) << mDecShift));
    }

    switch (entryY->mRotationInfo.mMaxFrame) {
    case 0:
        pTransform->mRotation.y = 0;
        break;
    case 1:
        pTransform->mRotation.y = static_cast<s16>(u16(u32(static_cast<s32>(mRotData[entryY->mRotationInfo.mOffset])) << mDecShift));
        break;
    default:
        pTransform->mRotation.y = static_cast<s16>(u16(u32(static_cast<s32>((int)J3DGetKeyFrameInterpolation(frame, &entryY->mRotationInfo, &mRotData[entryY->mRotationInfo.mOffset]))) << mDecShift));
    }

    switch (entryZ->mRotationInfo.mMaxFrame) {
    case 0:
        pTransform->mRotation.z = 0;
        break;
    case 1:
        pTransform->mRotation.z = static_cast<s16>(u16(u32(static_cast<s32>(mRotData[entryZ->mRotationInfo.mOffset])) << mDecShift));
        break;
    default:
        pTransform->mRotation.z = static_cast<s16>(u16(u32(static_cast<s32>((int)J3DGetKeyFrameInterpolation(frame, &entryZ->mRotationInfo, &mRotData[entryZ->mRotationInfo.mOffset]))) << mDecShift));
    }

    switch (entryX->mTranslateInfo.mMaxFrame) {
    case 0:
        pTransform->mTranslate.x = 0.0f;
        break;
    case 1:
        pTransform->mTranslate.x = mTransData[entryX->mTranslateInfo.mOffset];
        break;
    default:
        pTransform->mTranslate.x = J3DGetKeyFrameInterpolation(frame, &entryX->mTranslateInfo, &mTransData[entryX->mTranslateInfo.mOffset]);
    }

    switch (entryY->mTranslateInfo.mMaxFrame) {
    case 0:
        pTransform->mTranslate.y = 0.0f;
        break;
    case 1:
        pTransform->mTranslate.y = mTransData[entryY->mTranslateInfo.mOffset];
        break;
    default:
        pTransform->mTranslate.y = J3DGetKeyFrameInterpolation(frame, &entryY->mTranslateInfo, &mTransData[entryY->mTranslateInfo.mOffset]);
    }

    switch (entryZ->mTranslateInfo.mMaxFrame) {
    case 0:
        pTransform->mTranslate.z = 0.0f;
        break;
    case 1:
        pTransform->mTranslate.z = mTransData[entryZ->mTranslateInfo.mOffset];
        break;
    default:
        pTransform->mTranslate.z = J3DGetKeyFrameInterpolation(frame, &entryZ->mTranslateInfo, &mTransData[entryZ->mTranslateInfo.mOffset]);
    }
}
void J3DFrameCtrl::init(s16 endFrame) {
    mAttribute = EMode_LOOP;
    mState = 0;
    mStart = 0;
    mEnd = endFrame;
    mLoop = 0;
    mRate = 1.0f;
    mFrame = 0.0f;
}
void J3DFrameCtrl::update() {
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

