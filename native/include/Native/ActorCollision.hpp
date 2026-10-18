#pragma once
#include <Native/ArchiveResource.hpp>
#include <Native/CollisionCamera.hpp>
#include <memory>
#include <cmath>
#include <Game/Map/CollisionParts.hpp>
#include <Game/Map/KCollision.hpp>
#include <Game/Map/CollisionDirector.hpp>
#include <Game/Map/CollisionCategorizedKeeper.hpp>
#include <Game/LiveActor/HitSensor.hpp>
#include <Game/LiveActor/LiveActor.hpp>
#include <Game/Camera/CameraPolygonCodeUtil.hpp>
#include <Game/Util/SceneUtil.hpp>
namespace Native {
struct ActorCollisionDeleter{void operator()(CollisionParts* part)const{if(part){delete part->mServer->mapInfo;delete part->mServer;delete part;}}};
inline void initializeActorCollision(CollisionParts& part,JKRArchive& archive,const TPos3f& matrix,HitSensor* sensor,const void* kcl,const void* metadata,int keeper,int scaleType){
 if(!sensor||!sensor->mHost||scaleType<0||scaleType>2||keeper<0||keeper>=4||!part.mServer||!part.mServer->mapInfo)throw std::invalid_argument("Invalid actor collision initialization");
 for(int row=0;row<3;++row)for(int col=0;col<4;++col)if(!std::isfinite(matrix.mMtx[row][col]))throw std::invalid_argument("Nonfinite actor collision transform");
 TPos3f inverse;if(!PSMTXInverse(const_cast<MtxPtr>(matrix.mMtx),inverse.mMtx))throw std::invalid_argument("Singular actor collision transform");
 auto geometry=copyArchiveResource(archive,kcl);
 auto attributes=metadata?copyArchiveResource(archive,metadata):std::vector<std::uint8_t>{};
 part.mServer->setNativeData(geometry);
 if(metadata)part.mServer->mapInfo->attachNative(attributes);
 if(scaleType==0){part._CF=true;part._D0=false;}
 if(scaleType==1){part._CF=false;part._D0=true;}
 part.mHitSensor=sensor;part.resetAllMtx(matrix);
 TVec3f scale;part.mBaseMatrix.getScale(scale);
 part.mZone=MR::getCollisionDirector()->getCategoryKeeper(keeper)->getZone(MR::getCurrentPlacementZoneId());
 CollisionCameraCollection collection(sensor->mHost->mName,part.mZone->mZoneID);
 const bool attributesMatch=part.mServer->calcFarthestVertexDistance();if(metadata&&!attributesMatch)throw std::invalid_argument("Collision attributes do not match actor prisms");collection.commit();
 part.updateBoundingSphereRange(scale);part.mKeeperIndex=keeper;
}
}



