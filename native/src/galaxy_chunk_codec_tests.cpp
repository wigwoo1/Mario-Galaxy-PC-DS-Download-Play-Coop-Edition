#include <Game/System/GameDataGalaxyStorage.hpp>
#include <Game/System/GalaxyStatusAccessor.hpp>
#include <Game/System/BinaryDataContentAccessor.hpp>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <vector>
namespace MR{void copyMemory(void* d,const void* s,u32 n){std::memcpy(d,s,n);}}
const char* GalaxyStatusAccessor::getName()const{return "TestGalaxy";}
s32 GalaxyStatusAccessor::getPowerStarNum()const{return 8;}
// Constructor boundary: fixed descriptors instead of the live scenario service.
GameDataAllGalaxyStorage::GameDataAllGalaxyStorage(){
 mHeaderSerializer=new BinaryDataContentHeaderSerializer(new u8[32],32);mSomeGalaxyStorages.init(2);
 GalaxyStatusAccessor a;for(int i=0;i<2;++i)mSomeGalaxyStorages.push_back(new GameDataSomeGalaxyStorage(a));
 mSomeGalaxyStorages[0]->mGalaxyName="FirstGalaxy";mSomeGalaxyStorages[1]->mGalaxyName="SecondGalaxy";initHeaderSerializer();
}
int main(){
 GameDataAllGalaxyStorage source,target;
 for(int i=0;i<2;++i){auto p=source.getGalaxyStorage(i);p->mPowerStarOwnedFlags=u8(5+i);p->mAlreadyVisitedFlags=u8(10+i);for(int j=0;j<8;++j)p->mMaxCoinNum[j]=u16(256+i*100+j);}
 std::vector<u8> bytes(64,0xcc);assert(source.serialize(bytes.data()+1,62)==62);assert(bytes[0]==0xcc&&bytes[63]==0xcc);assert(bytes[1]==0&&bytes[2]==2);
 assert(target.deserialize(bytes.data()+1,62)==0);for(int i=0;i<2;++i){assert(target.getGalaxyStorage(i)->mPowerStarOwnedFlags==5+i);for(int j=0;j<8;++j)assert(target.getGalaxyStorage(i)->mMaxCoinNum[j]==256+i*100+j);}
 for(u32 len=0;len<62;++len){target.initializeData();assert(target.deserialize(bytes.data()+1,len)==-1);assert(target.getGalaxyStorage(0)->mPowerStarOwnedFlags==0);}
 auto duplicate=bytes;duplicate[43]=duplicate[23];duplicate[44]=duplicate[24];target.initializeData();assert(target.deserialize(duplicate.data()+1,62)==-1);assert(target.getGalaxyStorage(0)->mPowerStarOwnedFlags==0);
 auto bad=bytes;bad[22]=5;assert(target.deserialize(bad.data()+1,62)==-1);assert(target.getGalaxyStorage(0)->mPowerStarOwnedFlags==0);
 for(u32 len=0;len<62;++len){std::vector<u8> output(64,0xcc);assert(source.serialize(output.data()+1,len)==-1);for(auto b:output)assert(b==0xcc);}
 assert(source.getSignature()==0x47414c41&&source.makeHeaderHashCode()==0xbf0640ee);
 std::puts("PASS original galaxy chunk: BE layout, unaligned roundtrip, every truncation, duplicate record rollback, invalid extent and bounded write preservation");
}
