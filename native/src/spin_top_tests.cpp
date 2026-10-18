#include "Game/System/SpinDriverPathStorage.hpp"
#include "Game/System/GalaxyStatusAccessor.hpp"
#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdio>
static const char* currentName="First";
namespace MR{bool isNearZero(float x,float epsilon){return std::fabs(x)<epsilon;}u32 getHashCode(const char* name){return std::strcmp(name,"First")==0?0x1234:0x5678;}}
const char* GalaxyStatusAccessor::getName()const{return currentName;}
s32 GalaxyStatusAccessor::getScenarioNum()const{return 1;}
int main(){
 GalaxyStatusAccessor accessor;SpinDriverPathStorageGalaxy first(accessor);
 currentName="Second";SpinDriverPathStorageGalaxy second(accessor);
 SpinDriverPathStorage storage;storage.mGalaxyStorage.push_back(&first);storage.mGalaxyStorage.push_back(&second);
 float range;int a=storage.setup("First",1,1,2,&range);storage.updateValue("First",1,a,0.5f);
 int b=storage.setup("Second",1,2,3,&range);storage.updateValue("Second",1,b,1);
 u8 data[27];std::memset(data,0xcd,27);assert(storage.serialize(data+1,25)==25);
 const u8 expected[]={2,0x12,0x34,0,12,1,0,0,6,0xc1,0x82,0x80,0xff,0x56,0x78,0,12,1,0,0,6,0xc2,0x83,0,0xff};
 assert(!std::memcmp(data+1,expected,25)&&data[0]==0xcd&&data[26]==0xcd);
 for(unsigned n=0;n<25;++n){u8 out[25];std::memset(out,0xab,25);assert(storage.serialize(out,n)==-1);for(auto v:out)assert(v==0xab);assert(storage.deserialize(expected,n)==-1);assert(first.getScenarioStorage(1).mOneStorage[0].mDrawRange==0.5f);}
 u8 malformed[25];std::memcpy(malformed,expected,25);malformed[11]=0x40;malformed[24]=0;
 assert(storage.deserialize(malformed,25)==-1&&first.getScenarioStorage(1).mOneStorage[0].mDrawRange==0.5f);
 storage.initializeData();assert(storage.deserialize(data+1,25)==0);
 assert(first.getScenarioStorage(1).mOneStorage[0].mDrawRange==0.5f&&second.getScenarioStorage(1).mOneStorage[0].mDrawRange==1);
 std::memcpy(malformed,expected,25);malformed[13]=0xff;malformed[14]=0xff;
 assert(storage.deserialize(malformed,25)==1);
 assert(storage.deserialize(nullptr,25)==-1&&storage.serialize(nullptr,25)==-1);
 assert(storage.getSignature()==0x53504e31);
 delete[] first.mScenarioStorage;delete[] second.mScenarioStorage;
 std::puts("PASS SPN1 actual codec: complete two-galaxy bytes, all short reads/writes, cross-galaxy rollback, unaligned roundtrip and unknown compatibility");
}
