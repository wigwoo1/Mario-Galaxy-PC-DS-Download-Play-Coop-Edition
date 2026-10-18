#include <Game/Map/WaterInfo.hpp>
#include <cassert>
#include <cstdio>
int main(){
    WaterInfo info;
    assert(!info.isInWater() && info.mEdgeDistance==-1 && info.mSurfaceNormal.y==1);
    // Opaque owner markers are never dereferenced by WaterInfo.
    unsigned char marker=0;
    info.mWaterArea=reinterpret_cast<const WaterArea*>(&marker);
    assert(info.isInWater());info.clear();
    info.mOceanBowl=reinterpret_cast<const OceanBowl*>(&marker);
    assert(info.isInWater());info.clear();
    info.mOceanRing=reinterpret_cast<const OceanRing*>(&marker);
    assert(info.isInWater());info.clear();
    info.mOceanSphere=reinterpret_cast<const OceanSphere*>(&marker);
    info.mCamWaterDepth=19;info.mWaveHeight=2;info.mStreamVec.set(1,2,3);
    assert(info.isInWater());info.clear();
    assert(!info.isInWater() && info.mCamWaterDepth==0 && info.mWaveHeight==0);
    assert(info.mStreamVec.x==0 && info.mStreamVec.y==0 && info.mStreamVec.z==0);
    assert(info.mSurfaceNormal.x==0 && info.mSurfaceNormal.y==1 && info.mSurfaceNormal.z==0);
    std::puts("Original WaterInfo owner detection and reset passed");
}
