#include <cstdio>
#ifdef GALAXY_DIAGNOSTIC_CAMERA_ANIMATION
extern "C" void galaxyNativeBootTrace(const char*);
#endif
#include <Native/OwnedCameraAnimation.hpp>
#include <Native/CameraAnimationArchive.hpp>
#include <JSystem/JMath/JMath.hpp>
#include "Game/Camera/CameraAnim.hpp"
#include "Game/Camera/CamTranslatorAnim.hpp"
#include "Game/Camera/CameraLocalUtil.hpp"
#include "Game/Camera/CameraTargetObj.hpp"
#include "Game/Util/MathUtil.hpp"

void CameraAnim_FORCE_MATCH_SDATA2() {
    (void)1.0f;
    (void)0.0f;
    (void)MR::pi();
}

void CamAnmDataAccessor::set(void* pInfo, void* pValues) {
    mInfo = reinterpret_cast< CanmFrameInfo* >(pInfo);
    mValues = reinterpret_cast< f32* >(pValues);
}

void CamAnmDataAccessor::getPos(TVec3f* pPos, f32 key) const {
    pPos->set< f32 >(get(key, mInfo->mPosX.mOffset, mInfo->mPosX.mCount),  //
                     get(key, mInfo->mPosY.mOffset, mInfo->mPosY.mCount),  //
                     get(key, mInfo->mPosZ.mOffset, mInfo->mPosZ.mCount));
}

void CamAnmDataAccessor::getWatchPos(TVec3f* pWatchPos, f32 key) const {
    pWatchPos->set< f32 >(get(key, mInfo->mWatchPosX.mOffset, mInfo->mWatchPosX.mCount),
                          get(key, mInfo->mWatchPosY.mOffset, mInfo->mWatchPosY.mCount),
                          get(key, mInfo->mWatchPosZ.mOffset, mInfo->mWatchPosZ.mCount));
}

f32 CamAnmDataAccessor::getTwist(f32 key) const {
    CamnFrameComponentInfo& info = mInfo->mTwist;

    return get(key, info.mOffset, info.mCount);
}

f32 CamAnmDataAccessor::getFovy(f32 key) const {
    CamnFrameComponentInfo& info = mInfo->mFovy;

    return get(key, info.mOffset, info.mCount);
}

f32 CamAnmDataAccessor::get(f32 key, u32 offset, u32 count) const {
    u32 keyFloor = key;

    f32 rate = MR::clamp(key - keyFloor, 0.0f, 1.0f);

    if (keyFloor < count - 1) {
        return (1.0f - rate) * mValues[offset + keyFloor] + rate * mValues[offset + keyFloor + 1];
    } else {
        s32 idx = offset;
        idx += count;
        return getValue(idx - 1);
    }
}

void KeyCamAnmDataAccessor::set(void* pInfo, void* pValues) {
    mInfo = reinterpret_cast< CanmKeyFrameInfo* >(pInfo);
    mValues = reinterpret_cast< f32* >(pValues);
}

void KeyCamAnmDataAccessor::getPos(TVec3f* pPos, f32 key) const {
    pPos->set< f32 >(get(key, mInfo->mPosX.mOffset, mInfo->mPosX.mCount, mInfo->mPosX.mType),
                     get(key, mInfo->mPosY.mOffset, mInfo->mPosY.mCount, mInfo->mPosY.mType),
                     get(key, mInfo->mPosZ.mOffset, mInfo->mPosZ.mCount, mInfo->mPosZ.mType));
}

void KeyCamAnmDataAccessor::getWatchPos(TVec3f* pWatchPos, f32 key) const {
    pWatchPos->set< f32 >(get(key, mInfo->mWatchPosX.mOffset, mInfo->mWatchPosX.mCount, mInfo->mWatchPosX.mType),
                          get(key, mInfo->mWatchPosY.mOffset, mInfo->mWatchPosY.mCount, mInfo->mWatchPosY.mType),
                          get(key, mInfo->mWatchPosZ.mOffset, mInfo->mWatchPosZ.mCount, mInfo->mWatchPosZ.mType));
}

f32 KeyCamAnmDataAccessor::getTwist(f32 key) const {
    CanmKeyFrameComponentInfo& info = mInfo->mTwist;

    return get(key, info.mOffset, info.mCount, info.mType);
}

f32 KeyCamAnmDataAccessor::getFovy(f32 key) const {
    CanmKeyFrameComponentInfo& info = mInfo->mFovy;

    return get(key, info.mOffset, info.mCount, info.mType);
}

f32 KeyCamAnmDataAccessor::get(f32 key, u32 offset, u32 count, u32 type) const {
    if (count == 1) {
        return mValues[offset];
    }

    if (type == 0) {
        return get3f(key, offset, count);
    } else {
        return get4f(key, offset, count);
    }
}

u32 KeyCamAnmDataAccessor::searchKeyFrameIndex(f32 key, u32 offset, u32 count, u32 stride) const {
    u32 low = 0;
    u32 high = count;

    while (low < high) {
        u32 middle = (low + high) / 2;

        if (mValues[offset + middle * stride] <= key) {
            low = middle + 1;
        } else {
            high = middle;
        }
    }

    return low - 1;
}

f32 KeyCamAnmDataAccessor::get3f(f32 key, u32 offset, u32 count) const {
    if (key <= mValues[offset]) return mValues[offset + 1];
    const u32 last = offset + (count - 1) * 3;
    if (key >= mValues[last]) return mValues[last + 1];
    u32 index = searchKeyFrameIndex(key, offset, count, 3);
    f32* values = mValues + offset + index * 3;

    return calcHermite(key, values[0], values[1], values[2], values[3], values[4], values[5]);
}

f32 KeyCamAnmDataAccessor::get4f(f32 key, u32 offset, u32 count) const {
    if (key <= mValues[offset]) return mValues[offset + 1];
    const u32 last = offset + (count - 1) * 4;
    if (key >= mValues[last]) return mValues[last + 1];
    u32 index = searchKeyFrameIndex(key, offset, count, 4);
    f32* values = mValues + offset + index * 4;

    return calcHermite(key, values[0], values[1], values[3], values[4], values[5], values[6]);
}

f32 KeyCamAnmDataAccessor::calcHermite(f32 key, f32 tA, f32 pA, f32 mA, f32 tB, f32 pB, f32 mB) const {
    return JMAHermiteInterpolation(key, tA, pA, mA / 30.0f, tB, pB, mB / 30.0f);
}

CameraAnim::CameraAnim(const char* pName)
    : Camera(pName), _4C(), _50(1), mNrFrames(), mIsKey(), mSpeed(1.0f), mFileDataAccessor(), mDataAccessor(new CamAnmDataAccessor()),
      mKeyDataAccessor(new KeyCamAnmDataAccessor()), mNrValues(), _74(), mNativeAnimation(new Native::OwnedCameraAnimation()), mIsPaused() {
}

CameraAnim::~CameraAnim() { delete mNativeAnimation; delete mDataAccessor; delete mKeyDataAccessor; }

void CameraAnim::reset() {
    CameraLocalUtil::setFovy(this, CameraLocalUtil::getFovy(mCameraMan));
    CameraLocalUtil::setWatchPos(this, CameraLocalUtil::getWatchPos(mCameraMan));
    CameraLocalUtil::setPos(this, CameraLocalUtil::getPos(mCameraMan));
    CameraLocalUtil::setUpVec(this, CameraLocalUtil::getUpVec(mCameraMan));
    mIsPaused = false;
    mCurrentFrame = 0.0f;
}

CameraTargetObj* CameraAnim::calc() {
    if (!mNrFrames || !mFileDataAccessor) return nullptr;
    TPos3f mtx;
    mtx.identity();
    mtx.setXDir(CameraLocalUtil::getTarget(this)->getSideVec());
    mtx.setYDir(CameraLocalUtil::getTarget(this)->getUpVec());
    mtx.setZDir(CameraLocalUtil::getTarget(this)->getFrontVec());
    mtx.setTrans(CameraLocalUtil::getTarget(this)->getPosition());

    if (mNrFrames != 0 && mNrFrames > mCurrentFrame) {
        TVec3f pos;
        mFileDataAccessor->getPos(&pos, mCurrentFrame);
        mtx.mult(pos, pos);
        CameraLocalUtil::setPos(this, pos);

        TVec3f watchPos;
        mFileDataAccessor->getWatchPos(&watchPos, mCurrentFrame);
        mtx.mult(watchPos, watchPos);
        CameraLocalUtil::setWatchPos(this, watchPos);

        TVec3f up;
        mtx.getYDir(up);
        CameraLocalUtil::setUpVec(this, up);
        CameraLocalUtil::setWatchUpVec(this, up);

        CameraLocalUtil::setRoll(this, mFileDataAccessor->getTwist(mCurrentFrame) * MR::pi() / 180.0f);
        CameraLocalUtil::setFovy(this, mFileDataAccessor->getFovy(mCurrentFrame));

        if (!mIsPaused) {
            mCurrentFrame += mSpeed;
        }
    } else {
        const f32 frame = mNrFrames - 1;
        CameraLocalUtil::setRoll(this, mFileDataAccessor->getTwist(frame) * MR::pi() / 180.0f);
        CameraLocalUtil::setFovy(this, mFileDataAccessor->getFovy(mNrFrames - 1));
    }

    return nullptr;
}

void CameraAnim::setParam(u8* pFile, f32 speed) {
    if (!loadBin(pFile)) return;

    mSpeed = speed;
}

bool CameraAnim::isAnimEnd() const {
    return mNrFrames == 0 || mCurrentFrame >= mNrFrames;
}

u32 CameraAnim::getAnimFrame(u8* pFile) {
    Native::CameraAnimationData decoded;
    return Native::decodeMountedCameraAnimation(decoded,pFile) ? decoded.frames : 0;
}

CamTranslatorBase* CameraAnim::createTranslator() {
    return new CamTranslatorAnim(this);
}

bool CameraAnim::loadBin(u8* pFile) {
    Native::CameraAnimationData decoded;
    if (!Native::decodeMountedCameraAnimation(decoded,pFile)) {
#ifdef GALAXY_DIAGNOSTIC_CAMERA_ANIMATION
        galaxyNativeBootTrace("Camera animation rejected unowned or invalid resource");
#endif
        return false;
    }
    if (!mNativeAnimation->loadDecoded(std::move(decoded))) return false;
    mFileDataAccessor=mNativeAnimation->bind(*mDataAccessor,*mKeyDataAccessor);
    mIsKey=mNativeAnimation->keyed();
    mNrFrames=mNativeAnimation->frames();
    mNrValues=mNativeAnimation->valueCount();
    const auto& metadata=mNativeAnimation->metadata();
    std::memcpy(&_4C,&metadata[1],4);
    std::memcpy(&_50,&metadata[2],4);
#ifdef GALAXY_DIAGNOSTIC_CAMERA_ANIMATION
    char line[200];std::snprintf(line,sizeof(line),"Camera animation loaded resource=%p frames=%u keyed=%u values=%u",static_cast<void*>(pFile),unsigned(mNrFrames),unsigned(mIsKey),unsigned(mNrValues));
    galaxyNativeBootTrace(line);
#endif
    return true;
}
