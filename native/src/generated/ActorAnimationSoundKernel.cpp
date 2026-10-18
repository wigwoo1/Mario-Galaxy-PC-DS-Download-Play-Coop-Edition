#include <Game/Util/SoundUtil.hpp>
#include <Game/Util/LiveActorUtil.hpp>
#include <Game/Util/ModelUtil.hpp>
#include <Game/Util/MapUtil.hpp>
#include <Game/LiveActor/LiveActor.hpp>
#include <Game/LiveActor/Binder.hpp>
#include <Game/LiveActor/ModelManager.hpp>
#include <Game/System/ResourceHolder.hpp>
#include <Game/AudioLib/AudAnmSoundObject.hpp>
namespace MR {
    void startBas(const LiveActor* pActor, const char* pName, bool param3, f32 param4, f32 param5) {
        if (pActor->mSoundObject == nullptr) {
            return;
        }

        ResourceHolder* pResourceHolder = getResourceHolder(pActor);
        JAUSoundAnimation* pSoundAnimation = nullptr;

        if (pResourceHolder->mBasResTable->isExistRes(pName)) {
            pSoundAnimation = static_cast< JAUSoundAnimation* >(pResourceHolder->mBasResTable->getRes(pName));
        }

        pActor->mSoundObject->setMapCode(getMapSoundCodeFoot(pActor));
        pActor->mSoundObject->startAnimation(pSoundAnimation, param3, param4, param5);
    }
    void actorSoundMovement(LiveActor* pActor) {
        AudAnmSoundObject* pSoundObject = pActor->mSoundObject;

        if (pSoundObject == nullptr) {
            return;
        }

        pSoundObject->setMapCode(getMapSoundCodeFoot(pActor));

        if (pActor->mModelManager != nullptr) {
            ResourceHolder* pResourceHolder = getResourceHolder(pActor);

            if (pResourceHolder->mMotionResTable->mCount != 0) {
                f32 frame = getBckCtrl(pActor)->mFrame;

                pSoundObject->update(frame);
            }
        }

        pSoundObject->process();
    }
    s32 getMapSoundCodeFoot(const LiveActor* pActor) {
        const Binder* pBinder = pActor->mBinder;
        if (pBinder == nullptr) {
            return -1;
        }

        s32 groundCode = -1;
        s32 wallCode = -1;
        s32 roofCode = -1;
        const Triangle* pGround = &pBinder->mGroundInfo.mParentTriangle;
        if (pGround != nullptr) {
            groundCode = getSoundCodeIndex(pGround->getAttributes());
        } else if (&pBinder->mWallInfo.mParentTriangle != nullptr) {
            wallCode = getSoundCodeIndex(pBinder->mWallInfo.mParentTriangle.getAttributes());
        } else if (&pBinder->mRoofInfo.mParentTriangle != nullptr) {
            roofCode = getSoundCodeIndex(pBinder->mRoofInfo.mParentTriangle.getAttributes());
        }

        if (groundCode >= 0) {
            return groundCode;
        }

        if (roofCode >= 0) {
            return roofCode;
        }

        if (wallCode >= 0) {
            return wallCode;
        }

        return -1;
    }
}
