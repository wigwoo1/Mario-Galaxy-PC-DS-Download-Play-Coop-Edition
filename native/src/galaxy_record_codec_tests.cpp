#include <Game/System/GameDataGalaxyStorage.hpp>
#include <Game/System/GalaxyStatusAccessor.hpp>
#include <Game/System/BinaryDataContentAccessor.hpp>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <stdexcept>
const char* GalaxyStatusAccessor::getName() const{return "TestGalaxy";}
s32 GalaxyStatusAccessor::getPowerStarNum() const{return 8;}
int main(){
 u8 header[32]{};BinaryDataContentHeaderSerializer h(header,32);
 h.addAttribute("mGalaxyName",2);h.addAttribute("mPowerStarFlag",1);h.addAttribute("mFirstPlayFlag",1);h.addAttribute("mMaxCoinNum",16);h.flush();
 BinaryDataContentAccessor a(header);GalaxyStatusAccessor descriptor;GameDataSomeGalaxyStorage source(descriptor),target(descriptor);
 source.mPowerStarOwnedFlags=0xa5;source.mAlreadyVisitedFlags=0x5a;
 const u16 values[8]={0,1,255,256,999,0x1234,0x8000,65535};
 for(int i=0;i<8;++i)source.mMaxCoinNum[i]=values[i];
 u8 record[22];std::memset(record,0xcc,sizeof(record));source.serialize(a,record+1);
 assert(record[0]==0xcc&&record[21]==0xcc&&record[1]==0xcc&&record[2]==0xcc);
 assert(record[3]==0xa5&&record[4]==0x5a);
 for(int i=0;i<8;++i){assert(record[5+i*2]==u8(values[i]>>8));assert(record[6+i*2]==u8(values[i]));}
 assert(target.deserialize(a,record+1)==0&&target.mPowerStarOwnedFlags==0xa5&&target.mAlreadyVisitedFlags==0x5a);
 for(int i=0;i<8;++i)assert(target.mMaxCoinNum[i]==values[i]);
 header[19]=5; // Sixteen-byte field no longer fits in twenty-byte record.
 assert(target.deserialize(a,record+1)==-1&&target.mPowerStarOwnedFlags==0xa5);
 for(int i=0;i<8;++i)assert(target.mMaxCoinNum[i]==values[i]);
 u8 before[22];std::memcpy(before,record,22);bool rejected=false;
 try{source.serialize(a,record+1);}catch(const std::length_error&){rejected=true;}
 assert(rejected&&std::memcmp(before,record,22)==0);
 assert(target.deserialize(a,nullptr)==-1);
 header[19]=4;header[16]^=1; // Missing optional legacy coin attribute defaults to zero.
 assert(target.deserialize(a,record+1)==1&&target.mPowerStarOwnedFlags==0xa5);
 for(auto coin:target.mMaxCoinNum)assert(coin==0);
 std::puts("PASS original galaxy record: exact BE coins, unaligned roundtrip, canaries, incomplete-field atomic rejection and legacy defaults");
}
