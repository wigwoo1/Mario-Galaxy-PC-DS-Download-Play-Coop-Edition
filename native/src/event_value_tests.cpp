#include "Game/System/GameEventValueChecker.hpp"
#include <cassert>
#include <cstring>
#include <vector>
#include <cstdio>
namespace MR{
bool isEqualString(const char* a,const char* b){return std::strcmp(a,b)==0;}
u32 getHashCode(const char* name){u32 h=0;for(;*name;++name)h=h*31+u8(*name);return h;}
}
int main(){
 GameEventValueChecker values;
 const int index=values.findIndex("LuigiEventState");assert(index>=0);
 values.setValue("LuigiEventState",0x1234);
 std::vector<u8> bytes(values.mNumValues*4+2,0xcd);
 const auto n=values.serialize(bytes.data()+1,u32(bytes.size()-2));assert(n==values.mNumValues*4);
 assert(bytes.front()==0xcd&&bytes.back()==0xcd);
 assert(bytes[1+index*4+2]==0x12&&bytes[1+index*4+3]==0x34);
 const u16 hash=MR::getHashCode("LuigiEventState");
 assert(bytes[1+index*4]==u8(hash>>8)&&bytes[1+index*4+1]==u8(hash));
 for(int size=0;size<n;++size){std::vector<u8> shortOut(n,0xab);assert(values.serialize(shortOut.data(),size)==-1);for(auto b:shortOut)assert(b==0xab);}
 assert(values.serialize(nullptr,n)==-1);
 values.setValue("LuigiEventState",0);
 assert(values.deserialize(bytes.data()+1,n)==0);assert(values.getValue("LuigiEventState")==0x1234);
 for(unsigned size=1;size<=3;++size){assert(values.deserialize(bytes.data()+1,size)==-1);assert(values.getValue("LuigiEventState")==0x1234);}
 assert(values.deserialize(nullptr,4)==-1&&values.deserialize(nullptr,0)==0);
 const u8 repeated[]={u8(hash>>8),u8(hash),0x56,0x78,u8(hash>>8),u8(hash),0xab,0xcd};
 assert(values.deserialize(repeated,8)==0&&values.getValue("LuigiEventState")==0xabcd);
 assert(values.getSignature()==0x564c4531);
 delete[] values.mValues;values.mValues=nullptr;
 std::puts("PASS VLE1 original value table: BE hash/value, exact four-byte records, canaries, all short outputs, unaligned roundtrip, truncated/null rejection and duplicates");
}
