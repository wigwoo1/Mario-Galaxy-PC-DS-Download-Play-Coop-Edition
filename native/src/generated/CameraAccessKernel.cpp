#include <Game/LiveActor/MirrorCamera.hpp>
#include <Game/Util/CameraUtil.hpp>
#include <Game/Util/MathUtil.hpp>
#include <Game/Camera/CameraContext.hpp>
#include <Game/Camera/CameraDirector.hpp>
#include <Game/Scene/SceneObjHolder.hpp>
namespace {
    CameraContext* getCameraContext() {
        return MR::getSceneObj< CameraContext >(SceneObj_CameraContext);
    }
}
namespace MR {
    bool isExistMirrorCamera() {
        return MR::isExistSceneObj(SceneObj_MirrorCamera);
    }
    const TPos3f& getMirrorCameraViewMtx() {
        return getMirrorCamera()->mViewMtx;
    }
    const TPos3f& getMirrorModelTexMtx() {
        return getMirrorCamera()->mModelTexMtx;
    }
    const TPos3f& getCameraViewMtx() {
        return ::getCameraContext()->getViewMtx();
    }
    const TPos3f& getCameraInvViewMtx() {
        return ::getCameraContext()->getInvViewMtx();
    }
    const TProj3f& getCameraProjectionMtx() {
        return ::getCameraContext()->mProjection;
    }
    f32 getNearZ() {
        return ::getCameraContext()->mNearZ;
    }
    f32 getFarZ() {
        return ::getCameraContext()->mFarZ;
    }
    f32 getFovy() {
        return ::getCameraContext()->mFovy;
    }
    f32 calcCameraDistanceZ(const TVec3f& rPos) {
        const TPos3f& viewMtx = getCameraViewMtx();
        return MR::abs(rPos.x * viewMtx.get(2, 0) + rPos.y * viewMtx.get(2, 1) + rPos.z * viewMtx.get(2, 2) + 1.0f * viewMtx.get(2, 3));
    }
    bool isSubjectiveCameraOnForObjClipping() {
        return getCameraDirector()->mSubjectiveFrame > 0;
    }
}
