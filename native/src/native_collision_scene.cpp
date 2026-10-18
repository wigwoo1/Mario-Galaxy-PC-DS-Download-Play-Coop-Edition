#include "native_collision_scene.hpp"
#include <Native/CollisionCamera.hpp>
#include <Game/Map/CollisionDirector.hpp>
#include <Game/Map/CollisionCategorizedKeeper.hpp>
#include <Game/Map/CollisionCode.hpp>
#include <Game/NameObj/NameObjRegister.hpp>
#include <Game/NameObj/NameObjGroup.hpp>
#include <Game/Util/SingletonHolder.hpp>
#include <Game/Util/ObjUtil.hpp>
#include <Game/Util/SceneUtil.hpp>
#include <Game/Scene/SceneFunction.hpp>
#include <Game/System/GameSystem.hpp>
#include <Game/System/GameSystemSceneController.hpp>
#include <Game/NameObj/NameObjExecuteHolder.hpp>
#include <Game/Scene/StopSceneController.hpp>
#include <Game/Scene/SceneNameObjMovementController.hpp>
#include <Game/Camera/CameraPolygonCodeUtil.hpp>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <Game/Map/KCollision.hpp>
#ifdef GALAXY_ORIGINAL_MOVEMENT_CONTROLLER
#include <Game/Scene/SceneDataInitializer.hpp>
#include <Game/Effect/EffectSystem.hpp>
#include <Game/GameAudio/AudCameraWatcher.hpp>
#include <Game/GameAudio/AudEffectDirector.hpp>
#include <Game/GameAudio/AudBgmConductor.hpp>
#include <Game/Util/ShareUtil.hpp>
#include <Game/Player/PlayerEvent.hpp>
#include <Game/Scene/ScenePlayingResult.hpp>
#include <Game/Util/FurCtrl.hpp>
#include <Game/Scene/PlacementStateChecker.hpp>
#include <Game/Screen/ScreenAlphaCapture.hpp>
#include <Game/Screen/BloomEffect.hpp>
#include <Game/Screen/BloomEffectSimple.hpp>
#include <Game/Screen/DepthOfFieldBlur.hpp>
#include <Game/Screen/ImageEffectSystemHolder.hpp>
#include <Game/Screen/ScreenBlurEffect.hpp>
#include <Game/Screen/CinemaFrame.hpp>
#include <Game/Map/PlanetMapCreator.hpp>
#include <Game/Map/SphereSelector.hpp>
#include <Game/Scene/StageDataHolder.hpp>
#include <Game/NPC/EventDirector.hpp>
#include <Game/Camera/CameraDirector.hpp>
#include <Game/Camera/GameCameraCreator.hpp>
#include <Game/Screen/GameSceneLayoutHolder.hpp>
#include <Game/MapObj/StarPieceDirector.hpp>
#include <Game/Screen/SceneWipeHolder.hpp>
#include <Game/Map/NamePosHolder.hpp>
#include <Game/Screen/InformationObserver.hpp>
#include <Game/Screen/CometRetryButton.hpp>
#include <Game/Screen/CenterScreenBlur.hpp>
#include <Game/LiveActor/ShadowController.hpp>
#include <Game/LiveActor/ShadowVolumeDrawer.hpp>
#include <Game/LiveActor/ShadowSurfaceDrawer.hpp>
#include <Game/LiveActor/AllLiveActorGroup.hpp>
#include <Game/LiveActor/ClippingDirector.hpp>
#include <Game/Demo/DemoDirector.hpp>
#include <Game/LiveActor/MessageSensorHolder.hpp>
#include <Game/LiveActor/LiveActorGroupArray.hpp>
#include <Game/NameObj/MovementOnOffGroupHolder.hpp>
#include <Game/AreaObj/AreaObjContainer.hpp>
#include <Game/Screen/CaptureScreenDirector.hpp>
#include <Game/NPC/TalkDirector.hpp>
#include <Game/NPC/NPCDirector.hpp>
#include <Game/NPC/MiiFacePartsHolder.hpp>
#include <Game/NPC/MiiFaceIconHolder.hpp>
#include <Game/Player/MarioHolder.hpp>
#include <Game/Camera/CameraContext.hpp>
#include <Game/Util/BaseMatrixFollowTargetHolder.hpp>
#include <Game/Map/SleepControllerHolder.hpp>
#include <Game/Map/StageSwitch.hpp>
#include <Game/Map/SwitchWatcherHolder.hpp>
#include <revolution/gx/GXLighting.h>
#include <Game/Map/LightDirector.hpp>
#include <Game/Map/LightDataHolder.hpp>
#include <Game/Map/LightZoneDataHolder.hpp>
#include <Game/Map/LightPointCtrl.hpp>
#include <Game/LiveActor/SensorHitChecker.hpp>
#include <Game/Player/GroupChecker.hpp>
#include <Game/Gravity/PlanetGravityManager.hpp>
#include <Game/Util/HashUtil.hpp>
#endif
namespace { NativeCollisionScene* active=nullptr;
#ifdef GALAXY_ORIGINAL_MOVEMENT_CONTROLLER
constexpr int nameCapacity=0x1300; // Original GameSystemSceneController scene holder.
#else
constexpr int nameCapacity=32;
#endif
NativeCollisionScene& current(){if(!active)throw std::logic_error("No active native collision scene");return *active;}
void destroyTable(CodeTable* t){delete[] t->mHashTable;delete[] t->mCodeTable;delete[] t->mNameTable;delete t;}
}
NativeCollisionScene::NativeCollisionScene(int count):names(nameCapacity),zoneCount(count){
 if(active || count<1 || count>32)throw std::logic_error("Invalid or nested native collision scene");
 SingletonHolder<NameObjRegister>::init();auto* registry=SingletonHolder<NameObjRegister>::get();
 if(registry->mHolder)throw std::logic_error("Another native NameObj holder is active");
 active=this;registry->setCurrentHolder(&names);
 try{globals.create(SceneObj_CollisionDirector);}catch(...){registry->setCurrentHolder(nullptr);active=nullptr;throw;}
}
NativeCollisionScene::~NativeCollisionScene(){
 terrain.clear();
#ifdef GALAXY_ORIGINAL_MOVEMENT_CONTROLLER
 auto* light=static_cast<LightDirector*>(globals.getObj(SceneObj_LightDirector));
 if(light){
  if(light->mDataHolder){delete[] light->mDataHolder->mLights;delete light->mDataHolder;}
  if(light->mZoneDataHolder){for(int zone=0;zone<light->mZoneDataHolder->mCount;++zone)delete[] light->mZoneDataHolder->mZoneInfo[zone].mAreaInfo;delete[] light->mZoneDataHolder->mZoneInfo;delete light->mZoneDataHolder;}
  delete light->mPointCtrl;
  // Area/player controls and ResourceHolder belong to their respective owners.
  delete light;
 }
 // MarioHolder references the player; it does not own the MarioActor.
 // Gravity manager references stage-owned fields; it does not own them.
 delete globals.getObj(SceneObj_PlanetGravityManager);
 delete globals.getObj(SceneObj_MarioHolder);
 delete globals.getObj(SceneObj_CameraContext);
 delete globals.getObj(SceneObj_BaseMatrixFollowTargetHolder);
 delete globals.getObj(SceneObj_SleepControllerHolder);
 delete globals.getObj(SceneObj_StageSwitchContainer);
 delete globals.getObj(SceneObj_SwitchWatcherHolder);
 auto* checks=static_cast<GroupCheckManager*>(globals.getObj(SceneObj_GroupCheckManager));
 if(checks){for(auto* group:checks->mGroups){delete group->mHashTable;delete group;}delete checks;}
 auto* sensors=static_cast<SensorHitChecker*>(globals.getObj(SceneObj_SensorHitChecker));
 if(sensors){
  SensorGroup* groups[]={sensors->mPlayerGroup,sensors->mRideGroup,sensors->mEyeGroup,sensors->mSimpleGroup,sensors->mMapObjGroup,sensors->mCharacterGroup};
  for(auto* group:groups){delete[] group->mSensors;delete group;}
  // Groups reference actor-owned HitSensors; they do not own them.
  delete sensors;
 }
#endif
 auto* d=director();for(int i=0;i<4;++i){auto* k=d->mCategoryKeeper[i];for(auto* z:k->mZones)delete z;delete[] k->mHitInfoArray;delete k;}delete[] d->mCategoryKeeper;
 destroyTable(d->mCode->mFloorTable);destroyTable(d->mCode->mWallTable);destroyTable(d->mCode->mSoundTable);destroyTable(d->mCode->mCameraTable);delete d->mCode;delete d;
 names.clearArray();SingletonHolder<NameObjRegister>::get()->setCurrentHolder(nullptr);active=nullptr;
}
void NativeTerrainDeleter::operator()(CollisionParts* p)const {if(p){if(p->mZone&&std::find(p->mZone->mParts.begin(),p->mZone->mParts.end(),p)!=p->mZone->mParts.end())p->removeFromBelongZone();delete p->mServer->mapInfo;delete p->mServer;delete p;}}
CollisionParts& NativeCollisionScene::loadTerrain(const std::vector<std::uint8_t>& kcl,const std::vector<std::uint8_t>& metadata,const TPos3f& matrix,const char* hostName,int zone){
 if(active!=this || !hostName || zone<0 || zone>=zoneCount || cameraCollectionZone!=-1)throw std::logic_error("Invalid native terrain placement");
 for(int row=0;row<3;++row)for(int column=0;column<4;++column)if(!std::isfinite(matrix.mMtx[row][column]))throw std::invalid_argument("Nonfinite terrain transform");
 TPos3f inverse;if(!PSMTXInverse(const_cast<MtxPtr>(matrix.mMtx),inverse.mMtx))throw std::invalid_argument("Singular terrain transform");
 auto* keeper=director()->getCategoryKeeper(0);auto* target=keeper->getZone(zone);
 if(target->mParts.size()>=512)throw std::length_error("Native collision zone is full");
 std::unique_ptr<CollisionParts,NativeTerrainDeleter> part(new CollisionParts());
 part->mServer->setNativeData(kcl);part->mServer->mapInfo->attachNative(metadata);part->resetAllMtx(matrix);
 const auto priorGroups=pendingCameraGroups.size();
 try{
  MR::initCameraCodeCollection(hostName,zone);
  if(!part->mServer->calcFarthestVertexDistance())throw std::invalid_argument("Collision attributes do not match terrain prisms");
  MR::termCameraCodeCollection();
  TVec3f scale;matrix.getScale(scale);part->updateBoundingSphereRange(scale);
  part->mKeeperIndex=0;part->mZone=target;part->_CC=true;
  terrain.push_back(std::move(part));
 }catch(...){pendingCameraGroups.resize(priorGroups);cameraCollectionHost.clear();cameraCollectionZone=-1;cameraCollectionCodes.fill(false);throw;}
 auto& loaded=*terrain.back();loaded.addToBelongZone();return loaded;
}
CollisionDirector* NativeCollisionScene::director()const{return static_cast<CollisionDirector*>(globals.getObj(SceneObj_CollisionDirector));}
void NativeCollisionScene::finishPlacement(){
 if(active!=this)throw std::logic_error("Placement finalization requires active native scene");
#ifdef GALAXY_ORIGINAL_MOVEMENT_CONTROLLER
 auto* sleep=static_cast<SleepControllerHolder*>(globals.create(SceneObj_SleepControllerHolder));
#endif
 // Original GameScene performs this dispatch after actor placement and before sleep sync.
 names.callMethodAllObj(&NameObj::initAfterPlacement);
#ifdef GALAXY_ORIGINAL_MOVEMENT_CONTROLLER
 sleep->initSync();
#endif
}
void NativeCollisionScene::tick(){if(movementFlagsPending){names.syncWithFlags();movementFlagsPending=false;}for(auto* o:sensorMovement)o->executeMovement();for(auto* o:switchMovement)o->executeMovement();for(auto* o:movement)o->executeMovement();for(auto* o:mapMovement)o->executeMovement();}
NameObj* SceneObjHolder::newEachObj(int id){
 if(id==SceneObj_CollisionDirector)return new CollisionDirector();
#ifdef GALAXY_ORIGINAL_MOVEMENT_CONTROLLER
 if(id==SceneObj_SceneDataInitializer)return new SceneDataInitializer();
 if(id==SceneObj_EffectSystem)return new EffectSystem("エフェクトシステム",true);
 if(id==SceneObj_AudCameraWatcher)return new AudCameraWatcher();
 if(id==SceneObj_AudEffectDirector)return new AudEffectDirector();
 if(id==SceneObj_AudBgmConductor)return new AudBgmConductor();
 if(id==SceneObj_ResourceShare)return new ResourceShare();
 if(id==SceneObj_EventSequencer)return new EventSequencer();
 if(id==SceneObj_ScenePlayingResult)return new ScenePlayingResult();
 if(id==SceneObj_FurDrawManager)return new FurDrawManager(64);
 if(id==SceneObj_PlacementStateChecker)return new PlacementStateChecker("オブジェクト配置状態の監視");
 if(id==SceneObj_ScreenAlphaCapture)return new ScreenAlphaCapture("アルファテクスチャ取り込み");
 if(id==SceneObj_ImageEffectSystemHolder)return new ImageEffectSystemHolder();
 if(id==SceneObj_BloomEffect)return new BloomEffect("ブルーム");
 if(id==SceneObj_BloomEffectSimple)return new BloomEffectSimple();
 if(id==SceneObj_ScreenBlurEffect)return new ScreenBlurEffect("画面ブラー");
 if(id==SceneObj_DepthOfFieldBlur)return new DepthOfFieldBlur("被写界深度ブラー");
 if(id==SceneObj_CinemaFrame)return new CinemaFrame(true);
 if(id==SceneObj_PlanetMapCreator)return new PlanetMapCreator("惑星クリエイタ");
 if(id==SceneObj_SphereSelector)return new SphereSelector();
 if(id==SceneObj_StageDataHolder)return new StageDataHolder(MR::getCurrentStageName(),0,true);
 if(id==SceneObj_EventDirector)return new EventDirector();
 if(id==SceneObj_CameraDirector)return new CameraDirector("カメラ管理");
 if(id==SceneObj_GameSceneLayoutHolder)return new GameSceneLayoutHolder();
 if(id==SceneObj_StarPieceDirector)return new StarPieceDirector("スターピース指揮");
 if(id==SceneObj_SceneWipeHolder)return new SceneWipeHolder();
 if(id==SceneObj_NamePosHolder)return new NamePosHolder();
 if(id==SceneObj_InformationObserver)return new InformationObserver();
 if(id==SceneObj_CometRetryButton)return new CometRetryButton("コメットリトライボタン");
 if(id==SceneObj_CenterScreenBlur)return new CenterScreenBlur();
 if(id==SceneObj_ShadowControllerHolder)return new ShadowControllerHolder();
 if(id==SceneObj_ShadowVolumeDrawInit)return new ShadowVolumeDrawInit();
 if(id==SceneObj_ShadowSurfaceDrawInit)return new ShadowSurfaceDrawInit("水面影描画初期化");
 if(id==SceneObj_AllLiveActorGroup)return new AllLiveActorGroup();
 if(id==SceneObj_ClippingDirector)return new ClippingDirector();
 if(id==SceneObj_DemoDirector)return new DemoDirector("デモ指揮");
 if(id==SceneObj_MessageSensorHolder)return new MessageSensorHolder("システム汎用センサー");
 if(id==SceneObj_LiveActorGroupArray)return new LiveActorGroupArray("オブジェクトグループ");
 if(id==SceneObj_MovementOnOffGroupHolder)return new MovementOnOffGroupHolder("Movementグループ管理");
 if(id==SceneObj_AreaObjContainer)return new AreaObjContainer("エリアオブジェクトコンテナ管理");
 if(id==SceneObj_CaptureScreenActor)return new CaptureScreenActor(MR::DrawType_CaptureScreenIndirect,"Indirect");
 if(id==SceneObj_TalkDirector)return new TalkDirector("会話ディレクター");
 if(id==SceneObj_NPCDirector)return new NPCDirector();
 if(id==SceneObj_MiiFacePartsHolder)return new MiiFacePartsHolder(128);
 if(id==SceneObj_MiiFaceIconHolder)return new MiiFaceIconHolder(16,"Miiアイコン保持管理");
 if(id==SceneObj_NameObjExecuteHolder)return new NameObjExecuteHolder(4096);
 if(id==SceneObj_NameObjGroup)return new NameObjGroup("IgnorePauseNameObj",16);
 if(id==SceneObj_StopSceneController)return new StopSceneController();
 if(id==SceneObj_SceneNameObjMovementController)return new SceneNameObjMovementController();
 if(id==SceneObj_MarioHolder)return new MarioHolder();
 if(id==SceneObj_CameraContext)return new CameraContext();
 if(id==SceneObj_BaseMatrixFollowTargetHolder)return new BaseMatrixFollowTargetHolder("Base matrix follow targets",256,256);
 if(id==SceneObj_SleepControllerHolder)return new SleepControllerHolder();
 if(id==SceneObj_StageSwitchContainer)return new StageSwitchContainer();
 if(id==SceneObj_SwitchWatcherHolder)return new SwitchWatcherHolder();
 if(id==SceneObj_LightDirector)return new LightDirector();
 if(id==SceneObj_SensorHitChecker)return new SensorHitChecker("Sensor hit checker");
 if(id==SceneObj_GroupCheckManager)return new GroupCheckManager("Group checker");
 if(id==SceneObj_PlanetGravityManager)return new PlanetGravityManager("Planet gravity manager");
#endif
 throw std::logic_error("Unsupported native collision scene object");
}
namespace MR {
SceneObjHolder* getSceneObjHolder(){
 if(active)return &active->globals;
#ifdef GALAXY_ORIGINAL_MOVEMENT_CONTROLLER
 auto* system=SingletonHolder<GameSystem>::get();
 if(system && system->mSceneController && system->mSceneController->isExistSceneObjHolder())
  return system->mSceneController->getSceneObjHolder();
#endif
 return &current().globals;
}
#ifndef GALAXY_ORIGINAL_MOVEMENT_CONTROLLER
s32 getZoneNum(){return current().zoneCount;}
#endif
#ifndef GALAXY_ORIGINAL_MOVEMENT_CONTROLLER
void notifyRequestNameObjMovementOnOff(){current().movementFlagsPending=true;}
#endif
// Preview scenes retain camera groups locally; full game scenes use the
// original director and create the actual camera parameter chunks.
void initCameraCodeCollection(const char* name,s32 zone){
#ifdef GALAXY_ORIGINAL_MOVEMENT_CONTROLLER
 if(!active){MR::getCameraDirector()->initCameraCodeCollection(name,zone);return;}
#endif
 auto& s=current();if(!name || zone<0 || zone>=s.zoneCount || s.cameraCollectionZone!=-1)throw std::logic_error("Invalid camera code collection");
 s.cameraCollectionHost=name;s.cameraCollectionZone=zone;s.cameraCollectionCodes.fill(false);
}
void registerCameraCode(u32 code){
#ifdef GALAXY_ORIGINAL_MOVEMENT_CONTROLLER
 if(!active){MR::getCameraDirector()->registerCameraCode(code);return;}
#endif
 auto& s=current();if(s.cameraCollectionZone<0)throw std::logic_error("Camera code without collection");
 if(code<255)s.cameraCollectionCodes[code]=true;
}
void termCameraCodeCollection(){
#ifdef GALAXY_ORIGINAL_MOVEMENT_CONTROLLER
 if(!active){MR::getCameraDirector()->termCameraCodeCollection();return;}
#endif
 auto& s=current();if(s.cameraCollectionZone<0)throw std::logic_error("No camera code collection to finish");
 auto committed=s.pendingCameraGroups;
 for(unsigned code=0;code<255;++code)if(s.cameraCollectionCodes[code])committed.push_back({s.cameraCollectionHost,s.cameraCollectionZone,code});
 s.pendingCameraGroups.swap(committed);
 s.cameraCollectionCodes.fill(false);s.cameraCollectionHost.clear();s.cameraCollectionZone=-1;
}
void connectToScene(NameObj* o,int movement,int animation,int buffer,int draw){
 if(!active){
  MR::registerNameObjToExecuteHolder(o,movement,animation,buffer,draw);
  MR::connectToSceneTemporarily(o);
  MR::connectToDrawTemporarily(o);
  return;
 }
 auto& scene=current();
 if(std::find(scene.movement.begin(),scene.movement.end(),o)!=scene.movement.end() || std::find(scene.mapMovement.begin(),scene.mapMovement.end(),o)!=scene.mapMovement.end() || std::find(scene.switchMovement.begin(),scene.switchMovement.end(),o)!=scene.switchMovement.end() || std::find(scene.sensorMovement.begin(),scene.sensorMovement.end(),o)!=scene.sensorMovement.end())throw std::logic_error("Duplicate native movement registration");
 if(o && movement==MovementType_SensorHitChecker && animation==CalcAnimType_None && buffer==DrawBufferType_None && draw==DrawType_None){scene.sensorMovement.push_back(o);return;}
 if(o && movement==MovementType_SwitchWatcherHolder && animation==CalcAnimType_None && buffer==DrawBufferType_None && draw==DrawType_None){scene.switchMovement.push_back(o);return;}
 if(o && movement==MovementType_MapObj && animation==CalcAnimType_None && buffer==DrawBufferType_None && draw==DrawType_None){
  current().mapMovement.push_back(o);return;
 }
 if(!o || movement!=MovementType_CollisionDirector || animation!=CalcAnimType_None || buffer!=DrawBufferType_None || draw!=DrawType_None)throw std::logic_error("Unsupported native collision scheduling category");
 current().movement.push_back(o);
}
}



namespace Native {void abortCollisionCameraCollection() noexcept{
 if(active){active->cameraCollectionCodes.fill(false);active->cameraCollectionHost.clear();active->cameraCollectionZone=-1;return;}
#ifdef GALAXY_ORIGINAL_MOVEMENT_CONTROLLER
 auto* creator=MR::getCameraDirector()->mCameraCreator;
 std::fill(std::begin(creator->mCodes),std::end(creator->mCodes),u8(0));
 creator->mName=nullptr;creator->_8=-1;
#endif
}}

