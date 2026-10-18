#include "Game/System/StarPieceAlmsStorage.hpp"
#include <cassert>
#include <cstring>
#include <cstdio>
namespace MR{u32 getHashCode(const char*){return 42;}}
int main(){
 StarPieceAlmsStorage storage;
 storage.addValue(8,200);storage.addValue(8,500);assert(storage.getValue(8)==400);
 storage.addValue(8,-500);assert(storage.getValue(8)==0);
 storage.addValue(0,0x1234);storage.addValue(15,32767);
 u8 data[34];std::memset(data,0xcd,sizeof(data));
 assert(storage.serialize(data+1,32)==32);
 assert(data[0]==0xcd&&data[33]==0xcd&&data[1]==0x12&&data[2]==0x34&&data[31]==0x7f&&data[32]==0xff);
 for(unsigned n=0;n<32;++n){
 u8 output[32];std::memset(output,0xab,32);assert(storage.serialize(output,n)==-1);for(auto v:output)assert(v==0xab);
 assert(storage.deserialize(data+1,n)==-1);assert(storage.getValue(0)==0x1234);
 }
 assert(storage.serialize(nullptr,32)==-1&&storage.deserialize(nullptr,32)==-1);
 storage.initializeData();assert(storage.getValue(0)==0);
 assert(storage.deserialize(data+1,32)==0);assert(storage.getValue(0)==0x1234&&storage.getValue(15)==32767);
 assert(storage.getSignature()==0x50434531);
 std::puts("PASS PCE1: original donation clamps, BE16 values, canaries, all short inputs/outputs, null rejection and unaligned roundtrip");
}
