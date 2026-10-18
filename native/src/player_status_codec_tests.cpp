#include <Game/System/GameDataPlayerStatus.hpp>
#include <cassert>
#include <cstdio>
#include <cstring>
int main(){
 GameDataPlayerStatus status;
 assert(status.getPlayerLeft()==4&&status.mStockedStarPiece==0);
 status.mStoryProgress=3;status.mStockedStarPiece=0x01020304;status.mPlayerLeft=42;
 const u8 expected[]={3,1,2,3,4,0,42};
 u8 encoded[9];std::memset(encoded,0xcc,sizeof(encoded));
 assert(status.serialize(encoded+1,7)==7&&encoded[0]==0xcc&&encoded[8]==0xcc);
 assert(std::memcmp(encoded+1,expected,7)==0);
 GameDataPlayerStatus loaded;
 assert(loaded.deserialize(expected,7)==0&&loaded.mStoryProgress==3&&loaded.mStockedStarPiece==0x01020304);
 assert(loaded.getPlayerLeft()==4&&loaded.isPlayerLeftSupply());
 loaded.offPlayerLeftSupply();assert(!loaded.isPlayerLeftSupply());
 for(u32 size=0;size<7;++size){
  u8 untouched[8];std::memset(untouched,0xaa,8);
  assert(status.serialize(untouched,size)==-1);
  for(u8 byte:untouched)assert(byte==0xaa);
  if(size!=5){assert(loaded.deserialize(expected,size)==-1);assert(loaded.mStoryProgress==3&&loaded.mStockedStarPiece==0x01020304);}
 }
 assert(status.serialize(nullptr,7)==-1&&loaded.deserialize(nullptr,7)==-1);
 assert(loaded.deserialize(expected,5)==0&&!loaded.isPlayerLeftSupply()&&loaded.getPlayerLeft()==4);
 loaded.addPlayerLeft(200);assert(loaded.getPlayerLeft()==99);
 loaded.addPlayerLeft(-200);assert(loaded.getPlayerLeft()==0);
 loaded.mStockedStarPiece=0;loaded.addStockedStarPiece(20000);assert(loaded.mStockedStarPiece==9999);
 loaded.addStockedStarPiece(-20000);assert(loaded.mStockedStarPiece==0);
 assert(status.getSignature()==0x504c4159&&status.makeHeaderHashCode()==0x27c90f);
 std::puts("PASS original player status: exact big-endian PLAY payload, legacy five-byte records, supply/reset semantics, bounded atomic failure and limits");
}
