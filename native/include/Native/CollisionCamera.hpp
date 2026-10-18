#pragma once
#include <Game/Camera/CameraPolygonCodeUtil.hpp>
namespace Native {
void abortCollisionCameraCollection() noexcept;
class CollisionCameraCollection {
 bool pending=false;
public:
 CollisionCameraCollection(const char* name,int zone){MR::initCameraCodeCollection(name,zone);pending=true;}
 CollisionCameraCollection(const CollisionCameraCollection&)=delete;
 ~CollisionCameraCollection(){if(pending)abortCollisionCameraCollection();}
 void commit(){MR::termCameraCodeCollection();pending=false;}
};
}
