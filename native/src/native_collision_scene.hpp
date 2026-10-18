#pragma once
#include <Game/NameObj/NameObjHolder.hpp>
#include <Game/Scene/SceneObjHolder.hpp>
#include <vector>
#include <string>
#include <array>
#include <memory>
#include <Game/Map/CollisionParts.hpp>
class CollisionDirector;
struct NativeTerrainDeleter { void operator()(CollisionParts*) const; };
// Initial native scene host for original collision; unsupported services fail explicitly.
class NativeCollisionScene {
public:
 explicit NativeCollisionScene(int zoneCount);
 ~NativeCollisionScene();
 NativeCollisionScene(const NativeCollisionScene&)=delete;
 NativeCollisionScene& operator=(const NativeCollisionScene&)=delete;
 void tick();
 void finishPlacement();
 CollisionDirector* director() const;
 CollisionParts& loadTerrain(const std::vector<std::uint8_t>& kcl,const std::vector<std::uint8_t>& metadata,const TPos3f& matrix,const char* hostName,int zone);
 std::vector<std::unique_ptr<CollisionParts,NativeTerrainDeleter>> terrain;
 NameObjHolder names;
 SceneObjHolder globals;
 std::vector<NameObj*> movement;
 // Original MapObj movement follows CollisionDirector movement (0x22 vs 0x20).
 std::vector<NameObj*> mapMovement;
 std::vector<NameObj*> switchMovement;
 std::vector<NameObj*> sensorMovement;
 int zoneCount;
 bool movementFlagsPending=false;
 struct CameraGroup { std::string host; int zone; unsigned code; };
 std::vector<CameraGroup> pendingCameraGroups;
 std::string cameraCollectionHost;
 int cameraCollectionZone=-1;
 std::array<bool,255> cameraCollectionCodes{};
};
