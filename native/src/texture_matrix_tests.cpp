#include <Native/GxCommandState.hpp>
#include <cassert>
#include <cstdio>
int main(){
 Native::GxCommandState state;
 auto load=[&](unsigned address,const std::vector<float>& values){
  std::vector<unsigned char> bytes{0x10};
  unsigned header=((values.size()-1)<<16)|address;
  auto word=[&](unsigned v){for(int shift=24;shift>=0;shift-=8)bytes.push_back(v>>shift);};
  word(header);for(float v:values){unsigned bits;std::memcpy(&bits,&v,4);word(bits);}state.consume(bytes.data(),bytes.size());
 };
 load(30*4,{2,0,0,3,0,4,0,-1,0,0,5,2});
 auto uv=state.transformTexture(30,{.25f,.5f,1},false);assert((uv==std::array<float,3>{3.5f,1,1}));
 auto stq=state.transformTexture(30,{.25f,.5f,1},true);assert((stq==std::array<float,3>{3.5f,1,7}));
 load(0x500+3*4,{1,0,0,2,0,1,0,3,0,0,1,4});
 auto post=state.transformTexture(3,stq,true,true);assert((post==std::array<float,3>{5.5f,4,11}));
 assert((state.transformTexture(60,{.25f,.5f,1},false)==std::array<float,3>{.25f,.5f,1}));
 bool rejected=false;try{state.transformTexture(33,{0,0,1},true);}catch(const std::runtime_error&){rejected=true;}assert(rejected);
 rejected=false;try{state.transformTexture(31,{0,0,1},true);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
 load(36*4,{1,0,0,0,0,1,0,0});
 assert((state.transformTexture(36,{.2f,.3f,1},false)==std::array<float,3>{.2f,.3f,1}));
 rejected=false;try{state.transformTexture(36,{0,0,1},true);}catch(const std::runtime_error&){rejected=true;}assert(rejected);
 std::puts("PASS GX texture matrices: ST/STQ, identity, post transform and unloaded-row rejection");
}
