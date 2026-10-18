#include "Game/System/SpinDriverPathStorage.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdio>
namespace MR{bool isNearZero(float x,float epsilon){return std::fabs(x)<epsilon;}u32 getHashCode(const char*){return 0x1234;}}
const char* GalaxyStatusAccessor::getName()const{return "TestGalaxy";}
s32 GalaxyStatusAccessor::getScenarioNum()const{return 2;}
int main(){
 GalaxyStatusAccessor accessor;SpinDriverPathStorageGalaxy galaxy(accessor);float range;
 auto a=galaxy.setup(1,1,2,&range);galaxy.updateValue(1,a,0.5f);
 auto b=galaxy.setup(2,2,3,&range);galaxy.updateValue(2,b,1.0f);
 u8 bytes[20];std::memset(bytes,0xcd,20);assert(galaxy.serialize(bytes+1,18)==18);
 const u8 expected[]={0x12,0x34,0,18,2,0,0,6,0xc1,0x82,0x80,0xff,0,6,0xc2,0x83,0,0xff};
 assert(!std::memcmp(bytes+1,expected,18)&&bytes[0]==0xcd&&bytes[19]==0xcd);
 for(unsigned n=0;n<18;++n){u8 out[20];std::memset(out,0xab,20);assert(galaxy.serialize(out,n)==-1);for(auto v:out)assert(v==0xab);assert(galaxy.deserialize(expected,n)==-1);assert(galaxy.getScenarioStorage(1).mOneStorage[0].mDrawRange==0.5f);}
 u8 malformed[18];std::memcpy(malformed,expected,18);malformed[10]=0x40;malformed[17]=0;
 assert(galaxy.deserialize(malformed,18)==-1&&galaxy.getScenarioStorage(1).mOneStorage[0].mDrawRange==0.5f);
 galaxy.resetAllData();assert(galaxy.deserialize(bytes+1,18)==18);
 assert(galaxy.getScenarioStorage(1).mOneStorage[0].mDrawRange==0.5f&&galaxy.getScenarioStorage(2).mOneStorage[0].mDrawRange==1);
 delete[] galaxy.mScenarioStorage;galaxy.mScenarioStorage=nullptr;
 std::puts("PASS original launch-star galaxy codec: BE headers, two-scenario roundtrip, all truncations/short outputs and cross-scenario rollback");
}
