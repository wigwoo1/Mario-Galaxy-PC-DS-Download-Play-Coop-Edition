#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/AreaObj/AreaObjContainer.hpp"
#include "Game/Boss/BossAccessor.hpp"
#include "Game/Boss/SkeletalFishBabyRailHolder.hpp"
#include "Game/Boss/SkeletalFishBossRailHolder.hpp"
#include "Game/Boss/TripodBossAccesser.hpp"
#include "Game/Camera/CameraContext.hpp"
#include "Game/Camera/CameraDirector.hpp"
#include "Game/Demo/DemoDirector.hpp"
#include "Game/Demo/PrologueDirector.hpp"
#include "Game/Effect/EffectSystem.hpp"
#include "Game/Enemy/BegomanBase.hpp"
#include "Game/Enemy/KabokuriFireHolder.hpp"
#include "Game/Enemy/KameckBeamHolder.hpp"
#include "Game/Enemy/KarikariDirector.hpp"
#include "Game/Enemy/TakoHeiInkHolder.hpp"
#include "Game/GameAudio/AudBgmConductor.hpp"
#include "Game/GameAudio/AudCameraWatcher.hpp"
#include "Game/GameAudio/AudEffectDirector.hpp"
#include "Game/Gravity/PlanetGravityManager.hpp"
#include "Game/LiveActor/AllLiveActorGroup.hpp"
#include "Game/LiveActor/ClippingDirector.hpp"
#include "Game/LiveActor/LiveActorGroupArray.hpp"
#include "Game/LiveActor/MessageSensorHolder.hpp"
#include "Game/LiveActor/MirrorCamera.hpp"
#include "Game/LiveActor/SensorHitChecker.hpp"
#include "Game/LiveActor/ShadowController.hpp"
#include "Game/LiveActor/ShadowSurfaceDrawer.hpp"
#include "Game/LiveActor/ShadowVolumeDrawer.hpp"
#include "Game/LiveActor/VolumeModelDrawer.hpp"
#include "Game/Map/Air.hpp"
#include "Game/Map/CollisionDirector.hpp"
#include "Game/Map/LightDirector.hpp"
#include "Game/Map/NamePosHolder.hpp"
#include "Game/Map/OceanHomeMapCtrl.hpp"
#include "Game/Map/PlanetMapCreator.hpp"
#include "Game/Map/QuakeEffectGenerator.hpp"
#include "Game/Map/RaceManager.hpp"
#include "Game/Map/SleepControllerHolder.hpp"
#include "Game/Map/SphereSelector.hpp"
#include "Game/Map/StageSwitch.hpp"
#include "Game/Map/SunshadeMapHolder.hpp"
#include "Game/Map/SwitchWatcherHolder.hpp"
#include "Game/Map/WaterAreaHolder.hpp"
#include "Game/Map/WaterPlant.hpp"
#include "Game/MapObj/AirBubbleHolder.hpp"
#include "Game/MapObj/ArrowSwitchMultiHolder.hpp"
#include "Game/MapObj/BigBubbleHolder.hpp"
#include "Game/MapObj/BigFanHolder.hpp"
#include "Game/MapObj/ChipHolder.hpp"
#include "Game/MapObj/ClipAreaDropHolder.hpp"
#include "Game/MapObj/ClipAreaHolder.hpp"
#include "Game/MapObj/ClipFieldFillDraw.hpp"
#include "Game/MapObj/CoinHolder.hpp"
#include "Game/MapObj/CoinRotater.hpp"
#include "Game/MapObj/EarthenPipe.hpp"
#include "Game/MapObj/ElectricRailHolder.hpp"
#include "Game/MapObj/FallOutFieldDraw.hpp"
#include "Game/MapObj/FirePressureBulletHolder.hpp"
#include "Game/MapObj/GCapture.hpp"
#include "Game/MapObj/MapPartsRailGuideHolder.hpp"
#include "Game/MapObj/MiniatureGalaxyHolder.hpp"
#include "Game/MapObj/Note.hpp"
#include "Game/MapObj/PurpleCoinHolder.hpp"
#include "Game/MapObj/SpiderThread.hpp"
#include "Game/MapObj/SpinDriverPathDrawer.hpp"
#include "Game/MapObj/StarPieceDirector.hpp"
#include "Game/MapObj/WarpPod.hpp"
#include "Game/MapObj/WaterPressureBulletHolder.hpp"
#include "Game/NPC/EventDirector.hpp"
#include "Game/NPC/MiiFaceIconHolder.hpp"
#include "Game/NPC/MiiFacePartsHolder.hpp"
#include "Game/NPC/NPCDirector.hpp"
#include "Game/NPC/TalkDirector.hpp"
#include "Game/NameObj/MovementOnOffGroupHolder.hpp"
#include "Game/NameObj/NameObjExecuteHolder.hpp"
#include "Game/NameObj/NameObjGroup.hpp"
#include "Game/Player/GroupChecker.hpp"
#include "Game/Player/MarioHolder.hpp"
#include "Game/Player/PlayerEvent.hpp"
#include "Game/Ride/FluffWind.hpp"
#include "Game/Ride/PlantLeaf.hpp"
#include "Game/Ride/PlantStalk.hpp"
#include "Game/Ride/SwingRope.hpp"
#include "Game/Ride/Trapeze.hpp"
#include "Game/Scene/PlacementStateChecker.hpp"
#include "Game/Scene/SceneDataInitializer.hpp"
#include "Game/Scene/SceneNameObjMovementController.hpp"
#include "Game/Scene/ScenePlayingResult.hpp"
#include "Game/Scene/StageDataHolder.hpp"
#include "Game/Scene/StopSceneController.hpp"
#include "Game/Screen/BloomEffect.hpp"
#include "Game/Screen/BloomEffectSimple.hpp"
#include "Game/Screen/CaptureScreenDirector.hpp"
#include "Game/Screen/CenterScreenBlur.hpp"
#include "Game/Screen/CinemaFrame.hpp"
#include "Game/Screen/CometRetryButton.hpp"
#include "Game/Screen/DepthOfFieldBlur.hpp"
#include "Game/Screen/GalaxyMapController.hpp"
#include "Game/Screen/GalaxyNamePlateDrawer.hpp"
#include "Game/Screen/GameSceneLayoutHolder.hpp"
#include "Game/Screen/HeatHazeEffect.hpp"
#include "Game/Screen/ImageEffectSystemHolder.hpp"
#include "Game/Screen/InformationObserver.hpp"
#include "Game/Screen/LensFlare.hpp"
#include "Game/Screen/MoviePlayerSimple.hpp"
#include "Game/Screen/MoviePlayingSequence.hpp"
#include "Game/Screen/OdhConverter.hpp"
#include "Game/Screen/PlayerActionGuidance.hpp"
#include "Game/Screen/SceneWipeHolder.hpp"
#include "Game/Screen/ScreenAlphaCapture.hpp"
#include "Game/Screen/ScreenBlurEffect.hpp"
#include "Game/Screen/StaffRoll.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"
#include "Game/Util/FurCtrl.hpp"
#include "Game/Util/SceneUtil.hpp"
#include "Game/Util/ShareUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"











namespace MR {
    NameObj* createSceneObj(int id) {
        return getSceneObjHolder()->create(id);
    }



    bool isExistSceneObj(int id) {
        GameSystemSceneController* pSceneController = SingletonHolder< GameSystem >::get()->mSceneController;

        if (pSceneController == nullptr) {
            return false;
        }

        if (!pSceneController->isExistSceneObjHolder()) {
            return false;
        }

        return MR::getSceneObjHolder()->isExist(id);
    }
};  // namespace MR
