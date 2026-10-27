#include <Native/D3dModelDraw.hpp>
#include <Native/TevStage.hpp>
#include <cassert>
#include <cstdio>

static void bp(Native::GxCommandState& state,unsigned reg,unsigned value){
 const unsigned word=(reg<<24)|(value&0xffffff);
 const unsigned char bytes[]={0x61,(unsigned char)(word>>24),(unsigned char)(word>>16),(unsigned char)(word>>8),(unsigned char)word};
 state.consume(bytes,5);
}
static void color(Native::GxCommandState& state,unsigned id,bool konst,const std::array<int,4>& v){
 bp(state,0xe0+id*2,(konst?1u<<23:0)|(unsigned(v[0])&2047)|((unsigned(v[3])&2047)<<12));
 bp(state,0xe1+id*2,(konst?1u<<23:0)|(unsigned(v[2])&2047)|((unsigned(v[1])&2047)<<12));
}
int main(){
 using Microsoft::WRL::ComPtr;
 ComPtr<ID3D11Device> device;ComPtr<ID3D11DeviceContext> context;
 assert(SUCCEEDED(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,nullptr,&context)));
 Native::D3dEfb efb(*context.Get(),8,8);Native::D3dModelDraw first(*context.Get()),second(*context.Get());
 Native::GxCommandState state;bp(state,0x41,24);bp(state,0x40,0);bp(state,0xf3,(7<<16)|(7<<19));bp(state,0xf7,14);
 // RGB = signed register 1 + selected konst; alpha = signed register 1 alpha.
 bp(state,0xc0,(15<<12)|(14<<8)|(12<<4)|2|(1<<19));
 bp(state,0xc1,(7<<13)|(7<<10)|(7<<7)|(1<<4)|(1<<19));
 const std::vector<Native::ProjectedVertex> geometry={{{-1,-1,-.5f,1},{1,1,1,1}},{{0,1,-.5f,1},{1,1,1,1}},{{1,-1,-.5f,1},{1,1,1,1}}};
 const float clear[]={0,0,0,1};Native::TevFragmentInputs input{};
 auto cache=Native::detail::d3dModelShaderCache(device.Get());const auto initial=cache->pixelShaders.size();
 std::string uniformSource;size_t checked=0;
 auto checkPixels=[&](Native::D3dModelDraw& draw,const std::array<int,4>& expected){
  efb.clear(clear,1);draw.draw(efb,geometry);auto pixels=efb.readColor();
  for(unsigned c=0;c<4;++c)assert(pixels[(4*8+4)*4+c]==expected[c]);++checked;
 };
 for(unsigned i=0;i<1024;++i){
  color(state,1,false,{int(i%2048)-512,int((i*3)%2048)-1024,int((i*7)%2048)-1024,int(i%512)-128});
  for(unsigned k=0;k<4;++k)color(state,k,true,{int((i+k*17)&255),int((i*3+k*31)&255),int((i*7+k*47)&255),int((i*11+k*61)&255)});
  bp(state,0xf6,4|((i%32)<<4)|(((i/32)%32)<<9));
  auto source=Native::tevProgramHlsl(state,true);if(!i)uniformSource=source;else assert(source==uniformSource);
  auto expected=Native::evaluateTevProgram(state,input);first.setUntexturedMaterial(state);
  assert(cache->pixelShaders.size()==initial+1);
  if(i%17==0 || i==1023){
   color(state,1,false,{7,19,31,43});auto changed=Native::evaluateTevProgram(state,input);second.setUntexturedMaterial(state);
   checkPixels(first,expected);checkPixels(second,changed);checkPixels(first,expected);
  }
 }
 // A structural combiner change must still select a different program.
 bp(state,0xc0,(15<<12)|(14<<8)|(12<<4)|2|(1<<19)|(1<<18));
 first.setUntexturedMaterial(state);assert(cache->pixelShaders.size()==initial+2);
 checkPixels(first,Native::evaluateTevProgram(state,input));
 std::printf("PASS 1024 signed-color/konst states reuse one compiled pixel shader; %zu CPU-reference GPU pixel checks, per-draw snapshot isolation, and structural shader separation\n",checked);
}
