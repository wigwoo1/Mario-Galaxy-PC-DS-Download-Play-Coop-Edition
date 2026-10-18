#include "Game/System/SpinDriverPathStorage.hpp"
#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdio>
namespace MR{bool isNearZero(float x,float epsilon){return std::fabs(x)<epsilon;}}
int main(){
 SpinDriverPathStorageScenario scenario;float range=0;
 int index=scenario.setup(1,2,&range);assert(index==0&&range==0);
 scenario.updateValue(index,0.5f);
 unsigned char output[10];std::memset(output,0xcd,10);
 assert(scenario.serialize(output+1,8)==6);
 const unsigned char expected[]={0,6,0xc1,0x82,0x80,0xff};
 assert(!std::memcmp(output+1,expected,6)&&output[0]==0xcd&&output[7]==0xcd);
 for(unsigned n=0;n<6;++n){unsigned char dst[8];std::memset(dst,0xab,8);assert(scenario.serialize(dst,n)==-1);for(auto b:dst)assert(b==0xab);assert(scenario.deserialize(expected,n)==-1);assert(scenario.mOneStorage.size()==1&&scenario.mOneStorage[0].mDrawRange==0.5f);}
 const unsigned char malformed[]={0,6,0xc1,0x82,0x80,0};
 assert(scenario.deserialize(malformed,6)==-1&&scenario.mOneStorage[0].mDrawRange==0.5f);
 scenario.resetAllData();assert(scenario.deserialize(output+1,6)==6);
 assert(scenario.mOneStorage.size()==1&&scenario.mOneStorage[0].mZoneId==1&&scenario.mOneStorage[0].mSpinDriverId==2&&scenario.mOneStorage[0].mDrawRange==0.5f);
 std::puts("PASS original launch-star scenario block: BE size, quantized path, bounds/canaries, truncation and rollback");
}

