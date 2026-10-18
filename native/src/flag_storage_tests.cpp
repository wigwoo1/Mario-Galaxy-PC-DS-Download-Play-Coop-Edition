#include "Game/System/GameEventFlagStorage.hpp"
#include "Game/System/GameEventFlagTable.hpp"
#include "Game/Util/BitArray.hpp"
#include <cassert>
#include <cstring>
#include <cstdio>
static GameEventFlag flags[]={{"first",0,0,0,0,0,nullptr,nullptr},{"second",0,0,0,0,0,nullptr,nullptr},{"computed",0,1,0,0,0,nullptr,nullptr}};
bool GameEventFlagIter::isEnd()const{return mIter==3;}
void GameEventFlagIter::goNext(){++mIter;}
const GameEventFlag* GameEventFlagIter::getFlag()const{return flags+mIter;}
GameEventFlagAccessor::GameEventFlagAccessor(const GameEventFlag* f):mFlag(f){}
const char* GameEventFlagAccessor::getName()const{return mFlag->mName;}
namespace GameEventFlagTable{
s32 getTableSize(){return 3;}
int getIndex(const GameEventFlag* f){return int(f-flags);}
int getIndexFromHashCode(u16 h){if(h==0x1234)return 0;if(h==0x2345)return 1;if(h==0x3456)return 99;return -1;}
GameEventFlagIter getBeginIter(){return {0};}
}
namespace MR{
void zeroMemory(void* p,u32 n){std::memset(p,0,n);}
u32 getHashCode(const char* name){return std::strcmp(name,"first")==0?0x1234:0x2345;}
}
int main(){
 GameEventFlagStorage storage;storage.initializeData();storage.set(flags,true);
 unsigned char bytes[8];std::memset(bytes,0xcd,8);
 assert(storage.serialize(bytes+1,4)==4);
 const unsigned char expected[]={0xcd,0x92,0x34,0x23,0x45,0xcd,0xcd,0xcd};
 assert(std::memcmp(bytes,expected,8)==0);
 for(unsigned n=0;n<4;++n){std::memset(bytes,0xcd,8);assert(storage.serialize(bytes+1,n)==-1);for(auto b:bytes)assert(b==0xcd);}
 assert(storage.serialize(nullptr,4)==-1);
 const unsigned char valid[]={0x12,0x34,0xa3,0x45};
 assert(storage.deserialize(valid,4)==0);assert(!storage.isOn(flags)&&storage.isOn(flags+1));
 assert(storage.deserialize(valid,3)==-1);assert(!storage.isOn(flags)&&storage.isOn(flags+1));
 assert(storage.deserialize(nullptr,2)==-1);assert(storage.deserialize(nullptr,0)==0);
 const unsigned char rollback[]={0x92,0x34,0xb4,0x56};
 assert(storage.deserialize(rollback,4)==-1);assert(!storage.isOn(flags)&&storage.isOn(flags+1));
 const unsigned char unknown[]={0x92,0x34,0x7f,0xff};
 assert(storage.deserialize(unknown,4)==1);assert(storage.isOn(flags));
 const unsigned char duplicate[]={0x92,0x34,0x12,0x34};
 assert(storage.deserialize(duplicate,4)==0);assert(!storage.isOn(flags));
 assert(storage.getSignature()==0x464c4731);
 std::puts("PASS original FLG1 provider: big-endian records, computed exclusion, canaries, short/null/odd rejection, rollback, unknown and duplicate compatibility");
}
