#include <Native/CameraAnimationPointer.hpp>
#include "Game/Camera/CamTranslatorAnim.hpp"
#include "Game/Camera/CameraAnim.hpp"
#include "Game/Camera/CameraParamChunk.hpp"

void CamTranslatorAnim::setParam(const CameraParamChunk* pChunk) {
    CameraAnim* camera = mCamera;

    // mNum1 is used to store the pointer to the data
    camera->setParam(Native::resolveCameraAnimationData(pChunk->mGeneralParam->mNum1), pChunk->mGeneralParam->mDist);
}

Camera* CamTranslatorAnim::CamTranslatorAnim::getCamera() const {
    return mCamera;
}

u32 CamTranslatorAnim::getAnimFrame(const CameraParamChunk* pChunk) const {
    return CameraAnim::getAnimFrame(Native::resolveCameraAnimationData(pChunk->mGeneralParam->mNum1));
}