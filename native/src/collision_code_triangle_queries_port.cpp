#include <Game/Map/CollisionCode.hpp>
#include <Game/Map/HitInfo.hpp>

const char* CollisionCode::getFloorCodeString(const Triangle& triangle) {
    const u32 code = getCode(triangle.getAttributes(), mFloorTable, "Floor_code");
    return mFloorTable->getString(code);
}

const char* CollisionCode::getWallCodeString(const Triangle& triangle) {
    const u32 code = getCode(triangle.getAttributes(), mWallTable, "Wall_code");
    return mWallTable->getString(code);
}

const char* CollisionCode::getSoundCodeString(const Triangle& triangle) {
    return getSoundCodeString(triangle.getAttributes());
}

u32 CollisionCode::getCameraID(const Triangle& triangle) {
    return getCameraID(triangle.getAttributes());
}
