#include <Game/Speaker/SpkMixingBuffer.hpp>
#include <JSystem/JAudio2/JASCalc.hpp>
void SpkMixingBuffer::mix(s32 padChannel, s16* pWaveData, s32 readSize, f32 volume, s32 offset) {
    for (s32 i = 0; i < readSize; i++) {
        s32 sample = mSamples[padChannel][i + offset];
        s16 wav = pWaveData[i];

        sample += (s32)(wav * volume);

        mSamples[padChannel][i + offset] = JASCalc::clamp< s16, s32 >(sample);
    }
}
const s16* SpkMixingBuffer::getSamples(s32 idx) const {
    return mSamples[idx];
}
#include <Game/Speaker/SpkSound.hpp>
SpkSoundVolume::SpkSoundVolume() {
    mReleaseVolume = 1.0f;
    mReleaseRate = 0.0f;
    mFadeOutVolume = 1.0f;
    mFadeOutRate = 0.0f;
    mFadeVolume = 1.0f;
    mFadeTarget = 1.0f;
    mFadeRate = 0.0f;
    mBaseVolume = 1.0f;
}

void SpkSoundVolume::setRelease(s32 release) {
    if (release > 0) {
        mReleaseRate = 1.0f / release;
    } else {
        mReleaseRate = 1.0f;
    }

    mReleaseVolume = 1.0f;
}

f32 SpkSoundVolume::calc(bool& kill) {
    kill = false;
    if (mReleaseRate != 0.0f) {
        mReleaseVolume -= mReleaseRate;
        if (mReleaseVolume <= 0.0f) {
            mReleaseVolume = 0.0f;
            mReleaseRate = 0.0f;
            kill = true;
        }
    }

    if (mFadeOutRate != 0.0f) {
        mFadeOutVolume -= mFadeOutRate;
        if (mFadeOutVolume <= 0.0f) {
            mFadeOutVolume = 0.0f;
            mFadeOutRate = 0.0f;
            kill = true;
        }
    }

    if (mFadeRate != 0.0f) {
        mFadeVolume += mFadeRate;
        if (mFadeRate >= 0.0f) {
            if (mFadeVolume >= mFadeTarget) {
                mFadeVolume = mFadeTarget;
                mFadeRate = 0.0f;
            }
        } else {
            if (mFadeVolume <= mFadeTarget) {
                mFadeVolume = mFadeTarget;
                mFadeRate = 0.0f;
            }
        }
    }

    return mReleaseVolume * mReleaseVolume * mFadeOutVolume * mFadeVolume * mBaseVolume;
}

