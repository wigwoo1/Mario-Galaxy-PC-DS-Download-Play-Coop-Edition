#include "../native_collision_scene.hpp"
#include <Native/CollisionCamera.hpp>
#include <Game/Map/CollisionDirector.hpp>
#include <Game/Map/CollisionCategorizedKeeper.hpp>
#include <Game/Map/CollisionCode.hpp>
#include <Game/NameObj/NameObjRegister.hpp>
#include <Game/Util/SingletonHolder.hpp>
#include <Game/Util/ObjUtil.hpp>
#include <Game/Util/SceneUtil.hpp>
#include <Game/Scene/SceneFunction.hpp>
#include <Game/Camera/CameraPolygonCodeUtil.hpp>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <Game/Map/KCollision.hpp>
#ifdef GALAXY_ORIGINAL_MOVEMENT_CONTROLLER
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

namespace MR {

#ifndef GALAXY_ORIGINAL_MOVEMENT_CONTROLLER
s32 getZoneNum(){return current().zoneCount;}
#endif
#ifndef GALAXY_ORIGINAL_MOVEMENT_CONTROLLER
void notifyRequestNameObjMovementOnOff(){current().movementFlagsPending=true;}
#endif
// Preserve camera group IDs until the original camera chunk factory is linked.




}



namespace Native {void abortCollisionCameraCollection() noexcept{if(active){active->cameraCollectionCodes.fill(false);active->cameraCollectionHost.clear();active->cameraCollectionZone=-1;}}}


