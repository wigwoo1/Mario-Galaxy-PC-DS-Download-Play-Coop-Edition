#include <Native/GxCommandState.hpp>
#include <cassert>
#include <cstdio>
int main(){
 Native::GxCommandState state;Native::GpuResources resources;
 auto cp=[&](u8 reg,u32 value){u8 b[]={8,reg,u8(value>>24),u8(value>>16),u8(value>>8),u8(value)};state.consume(b,6);};
 cp(0x50,(2<<13)|(3<<15));
 std::vector<std::vector<u8>> encoded={{0xf8,0x1f},{13,71,149},{13,71,149,2},{0x12,0x34},{0x04,0x20,0xc4},{13,71,149,227}};
 std::array<std::array<unsigned,4>,6> expected={{{255,0,255,255},{13,71,149,255},{13,71,149,255},{17,34,51,68},{4,8,12,16},{13,71,149,227}}};
 for(unsigned f=0;f<6;++f){
  auto address=resources.add(encoded[f].data(),encoded[f].size());cp(0xa2,address);cp(0xa3,address);cp(0xb2,4);cp(0xb3,4);cp(0x70,(f<<14)|(f<<18));
  assert(state.indexedColor(0,0,0,resources)==expected[f]);assert(state.indexedColor(0,1,0,resources)==expected[f]);
  bool rejected=false;try{state.indexedColor(0,0,256,resources);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
  resources.remove(encoded[f].data());
 }
 std::puts("PASS indexed GX vertex colors: RGB565/RGB8/RGBX8/RGBA4/RGBA6/RGBA8 on both channels");
}
