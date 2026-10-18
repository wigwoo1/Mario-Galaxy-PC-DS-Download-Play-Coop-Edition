#include <Native/D3dModelDraw.hpp>
#include "native_model_texture.hpp"
#include "native_model_projection.hpp"
#include "depth_texture_gpu_tests.hpp"
#include <cassert>
#include <cstdio>
std::vector<Native::ProjectedVertex> triangle(float z,bool red){
 return {{{-.8f,-.8f,z,1},{red?1.f:0,0,red?0.f:1,1}},{{0,.8f,z,1},{red?1.f:0,0,red?0.f:1,1}},{{.8f,-.8f,z,1},{red?1.f:0,0,red?0.f:1,1}}};
}
int main(){
 Microsoft::WRL::ComPtr<ID3D11Device> device;Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
 assert(SUCCEEDED(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,nullptr,&context)));
 Native::D3dEfb efb(*context.Get(),64,64);Native::D3dModelDraw draw(*context.Get());float clear[4]={0,0,0,1};efb.clear(clear,1);
 draw.draw(efb,triangle(-.2f,false));draw.draw(efb,triangle(-.8f,true));draw.draw(efb,triangle(-.2f,false));
 auto pixels=efb.readColor();auto center=(32*64+32)*4;assert(pixels[center]==255&&pixels[center+2]==0&&pixels[0]==0);
 auto depthValues=efb.readDepth();assert(depthValues.size()==64*64&&std::fabs(depthValues[32*64+32]-.2f)<.00001f&&depthValues[0]==1);
 efb.clear(clear,1);draw.draw(efb,triangle(.1f,true));pixels=efb.readColor();assert(pixels[center]==0);
 Native::GxCommandState material;
 auto bp=[&](unsigned reg,unsigned value){unsigned word=(reg<<24)|(value&0xffffff);unsigned char command[]={0x61,(unsigned char)(word>>24),(unsigned char)(word>>16),(unsigned char)(word>>8),(unsigned char)word};material.consume(command,5);};
 bp(0x41,24);
 bp(0x40,23);
 bp(0xf3,(7<<16)|(7<<19)); // Explicit GX ALWAYS/ALWAYS alpha test.
 bp(0xf6,4);bp(0xf7,14); // Identity R/G and B/A swap table.
 // RGB: raster * one + half bias, clamped. Alpha: raster alpha.
 bp(0xc0,(15<<12)|(10<<8)|(12<<4)|15|(1<<16)|(1<<19));
 bp(0xc1,(7<<13)|(7<<10)|(7<<7)|(5<<4)|(1<<19));
 draw.setUntexturedMaterial(material);efb.clear(clear,1);draw.draw(efb,triangle(-.5f,true));
 pixels=efb.readColor();assert(pixels[center]==255&&pixels[center+1]==128&&pixels[center+2]==128&&pixels[center+3]==255);
 bp(0x28,64);bool rejected=false;try{draw.setUntexturedMaterial(material);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
 // Rejection preserves the preceding valid pixel shader.
 efb.clear(clear,1);draw.draw(efb,triangle(-.5f,true));pixels=efb.readColor();assert(pixels[center+1]==128);
 bp(0x28,0);
 auto compare=[](unsigned f,int a,int b){switch(f){case 0:return false;case 1:return a<b;case 2:return a==b;case 3:return a<=b;case 4:return a>b;case 5:return a!=b;case 6:return a>=b;default:return true;}};
 unsigned alphaChecks=0;
 for(unsigned fn=0;fn<8;++fn)for(unsigned operation=0;operation<4;++operation){
  bp(0xf3,128|(128<<8)|(fn<<16)|(2<<19)|(operation<<22));
  draw.setUntexturedMaterial(material);
  for(int alpha:{127,128,129}){
   auto geometry=triangle(-.5f,true);for(auto& vertex:geometry)vertex.color[3]=float(alpha)/255.f;
   efb.clear(clear,1);draw.draw(efb,geometry);pixels=efb.readColor();
   bool a=compare(fn,alpha,128),b=alpha==128;
   bool pass=operation==0?a&&b:operation==1?a||b:operation==2?a!=b:a==b;
   assert((pixels[center]==255)==pass);++alphaChecks;
  }
 }
 // Discarded near fragment must not prevent a later farther accepted draw.
 bp(0xf3,(0<<16)|(0<<19));draw.setUntexturedMaterial(material);efb.clear(clear,1);draw.draw(efb,triangle(-.8f,true));
 bp(0x41,24);
 bp(0x40,23);
 bp(0xf3,(7<<16)|(7<<19));draw.setUntexturedMaterial(material);draw.draw(efb,triangle(-.2f,false));pixels=efb.readColor();assert(pixels[center+2]==255);
 TextureImage texture{2,1,6,{{13,71,149,255},{29,83,227,128}}};texture.minFilter=texture.magFilter=0;texture.wrapS=texture.wrapT=0;
 NativeModelTexture uploaded(*device.Get(),texture);
 std::array<ID3D11ShaderResourceView*,8> views{};std::array<ID3D11SamplerState*,8> samplers{};
 views[3]=uploaded.view.Get();samplers[3]=uploaded.sampler.Get();
 bp(0x28,64|3);bp(0xc0,(15<<12)|(8<<8)|(12<<4)|15|(1<<19));bp(0xc1,(7<<13)|(7<<10)|(7<<7)|(4<<4)|(1<<19));
 draw.setMaterial(material,views,samplers);
 for(unsigned texel=0;texel<2;++texel){
  auto geometry=triangle(-.5f,true);
  for(auto& vertex:geometry){vertex.uv[0]=texel?1.5f:.5f;vertex.uv[1]=1;vertex.uv[2]=2;}
  efb.clear(clear,1);draw.draw(efb,geometry);pixels=efb.readColor();auto expected=texture.pixels[texel];
  assert(pixels[center]==expected.r&&pixels[center+1]==expected.g&&pixels[center+2]==expected.b&&pixels[center+3]==expected.a);
 }
 // The same texture must select independently authored coordinates 1 and 7.
 for(unsigned coord:{1u,7u}){
  bp(0x28,64|3|(coord<<3));draw.setMaterial(material,views,samplers);
  auto geometry=triangle(-.5f,true);
  for(auto& vertex:geometry){vertex.uv[0]=.25f;vertex.uv[1]=.5f;vertex.uv[2]=1;vertex.extraUv[coord-1][0]=1.5f;vertex.extraUv[coord-1][1]=1;vertex.extraUv[coord-1][2]=2;}
  efb.clear(clear,1);draw.draw(efb,geometry);pixels=efb.readColor();assert(pixels[center]==29&&pixels[center+1]==83&&pixels[center+2]==227&&pixels[center+3]==128);
 }
 Native::D3dModelDraw depthDraw(*context.Get());
 // Actual startup material uses indirect map 1 / coordinate 1, bias STU,
 // and matrix 0 to offset the regular texture. Keep the two sources distinct.
 {
  TextureImage pattern{4,1,6,{{10,20,30,255},{60,70,80,255},{110,120,130,255},{160,170,180,255}}};
  pattern.minFilter=pattern.magFilter=0;pattern.wrapS=pattern.wrapT=0;
  TextureImage displacement{1,1,6,{{213,128,128,129}}};
  displacement.minFilter=displacement.magFilter=0;
  NativeModelTexture target(*device.Get(),pattern),offset(*device.Get(),displacement);
  auto iviews=views;auto isamplers=samplers;
  iviews[3]=target.view.Get();isamplers[3]=target.sampler.Get();
  iviews[1]=offset.view.Get();isamplers[1]=offset.sampler.Get();
  bp(0,(1<<16)|2);bp(0x28,64|3);bp(0x27,1|(1<<3));bp(0x25,0);
  bp(0x30,3);bp(0x31,0);bp(0x32,0);bp(0x33,0);
  // .5 coefficient, exponent +1 => one texel for biased S=1.
  bp(6,512|(2<<22));bp(7,0);bp(8,1<<22);
  bp(0x10,(7<<4)|(1<<9));
  auto geometry=triangle(-.5f,true);
  for(auto& vertex:geometry){vertex.uv[0]=.125f;vertex.uv[1]=.5f;vertex.uv[2]=1;vertex.extraUv[0][0]=vertex.extraUv[0][1]=.5f;vertex.extraUv[0][2]=1;}
  draw.setMaterial(material,iviews,isamplers);efb.clear(clear,1);draw.draw(efb,geometry);pixels=efb.readColor();
  assert(pixels[center]==60&&pixels[center+1]==70&&pixels[center+2]==80);
  auto checkTexel=[&](unsigned index){draw.setMaterial(material,iviews,isamplers);efb.clear(clear,1);draw.draw(efb,geometry);pixels=efb.readColor();assert(pixels[center]==pattern.pixels[index].r&&pixels[center+1]==pattern.pixels[index].g&&pixels[center+2]==pattern.pixels[index].b);};
  // All three matrices, signed coefficient and exponent in both directions.
  for(unsigned matrix=1;matrix<=3;++matrix){unsigned base=6+(matrix-1)*3;bp(base,(2048-512)|(2<<22));bp(base+1,0);bp(base+2,1<<22);bp(0x10,(7<<4)|(matrix<<9));for(auto& vertex:geometry)vertex.uv[0]=.625f;checkTexel(1);}
  bp(6,512|(2<<22));bp(7,0);bp(8,1<<22);for(auto& vertex:geometry)vertex.uv[0]=.125f;
  // Lower-bit indirect formats shift the high bits and add one for bias.
  for(unsigned format=1;format<4;++format){
   displacement.pixels[0].a=1u<<(format+2);NativeModelTexture reduced(*device.Get(),displacement);
   iviews[1]=reduced.view.Get();isamplers[1]=reduced.sampler.Get();
   bp(6,512|(1<<22));bp(8,1<<22);bp(0x10,(format<<2)|(7<<4)|(1<<9));checkTexel(1);
  }
  iviews[1]=offset.view.Get();isamplers[1]=offset.sampler.Get();
  // S/T matrices multiply the incoming coordinate by one sampled component.
  bp(6,3<<22);bp(7,0);bp(8,1<<22);
  for(unsigned matrix:{5u,9u}){bp(0x10,matrix<<9);checkTexel(1);}
  // Wrap and add-previous apply even without an enabled indirect source.
  bp(0,2);
  for(unsigned wrap=1;wrap<=6;++wrap){
   for(auto& vertex:geometry)vertex.uv[0]=wrap<6?float((256u>>(wrap-1))+.5f)/4.f:.625f;
   bp(0x10,(wrap<<13)|(6<<16));checkTexel(0);
  }
  for(auto& vertex:geometry)vertex.uv[0]=.625f;bp(0x10,(1<<9));checkTexel(2); // Missing indirect source cannot supply an offset.
  bp(0,(1<<16)|(1<<10)|2);bp(0x28,64|3|((64|3)<<12));
  bp(0xc2,material.bp(0xc0));bp(0xc3,material.bp(0xc1));
  for(auto& vertex:geometry)vertex.uv[0]=.125f;
  bp(6,512|(2<<22));bp(7,0);bp(8,1<<22);bp(0x10,(7<<4)|(1<<9));bp(0x11,1<<20);checkTexel(2);
  bp(0,(1<<16)|2);bp(0x10,(7<<4)|(1<<9)|(1<<19));checkTexel(1); // Explicit unmodified-LOD shader path.
  auto missing=iviews;missing[1]=nullptr;bool missingOffset=false;try{draw.setMaterial(material,missing,isamplers);}catch(const std::invalid_argument&){missingOffset=true;}assert(missingOffset);
  bp(0x10,(1<<7));bool bumpRejected=false;try{draw.setMaterial(material,iviews,isamplers);}catch(const std::invalid_argument&){bumpRejected=true;}assert(bumpRejected);
  TextureImage square{2,2,6,{{1,2,3,255},{4,5,6,255},{7,8,9,255},{10,11,12,255}}};square.minFilter=square.magFilter=0;
  displacement.pixels[0]={213,128,129,128};
  NativeModelTexture squareTarget(*device.Get(),square),verticalOffset(*device.Get(),displacement);
  iviews[3]=squareTarget.view.Get();isamplers[3]=squareTarget.sampler.Get();iviews[1]=verticalOffset.view.Get();isamplers[1]=verticalOffset.sampler.Get();
  bp(0x30,1);bp(0x31,1);bp(6,2<<22);bp(7,512<<11);bp(8,1<<22);bp(0x10,(7<<4)|(1<<9));
  for(auto& vertex:geometry){vertex.uv[0]=vertex.uv[1]=.25f;}
  draw.setMaterial(material,iviews,isamplers);efb.clear(clear,1);draw.draw(efb,geometry);pixels=efb.readColor();assert(pixels[center]==7&&pixels[center+1]==8&&pixels[center+2]==9);
  bp(7,(2048-512)<<11);for(auto& vertex:geometry)vertex.uv[1]=.75f;
  draw.setMaterial(material,iviews,isamplers);efb.clear(clear,1);draw.draw(efb,geometry);pixels=efb.readColor();assert(pixels[center]==1&&pixels[center+1]==2&&pixels[center+2]==3);
  std::puts("PASS indirect texture uses separate map/UV, ABG channels, signed bias, matrix and exponent to displace sampled texel");
  std::puts("PASS indirect coordinate modes: three signed matrices, reduced formats, S/T, wraps, add-previous, missing-stage handling and explicit bump-alpha rejection");
  std::puts("PASS indirect matrix second row: blue-channel displacement and positive/negative vertical offsets");
  bp(0,0);bp(0x10,0);
 }
 unsigned depthChecks=0;
 for(unsigned fn=0;fn<8;++fn)for(int relation:{-1,0,1}){
  efb.clear(clear,1);bp(0x40,1|(7<<1)|16);depthDraw.setDepthMode(material);depthDraw.draw(efb,triangle(-.5f,false));
  bp(0x40,1|(fn<<1)|16);depthDraw.setDepthMode(material);depthDraw.draw(efb,triangle(-.5f+relation*.125f,true));
  pixels=efb.readColor();bool pass=compare(fn,relation,0);assert((pixels[center]==255)==pass);assert((pixels[center+2]==255)==!pass);++depthChecks;
 }
 efb.clear(clear,1);bp(0x40,1|(3<<1));depthDraw.setDepthMode(material);depthDraw.draw(efb,triangle(-.8f,true));
 bp(0x41,24);
 bp(0x40,23);depthDraw.setDepthMode(material);depthDraw.draw(efb,triangle(-.2f,false));pixels=efb.readColor();assert(pixels[center+2]==255);
 bp(0x40,16);depthDraw.setDepthMode(material);depthDraw.draw(efb,triangle(-.1f,true));pixels=efb.readColor();assert(pixels[center]==255);
 unsigned cullChecks=0;
 bp(0x40,0);depthDraw.setDepthMode(material);
 for(unsigned mode=0;mode<4;++mode)for(unsigned reverse=0;reverse<2;++reverse){
  bp(0,mode<<14);depthDraw.setCullMode(material);
  auto geometry=triangle(-.5f,true);if(reverse)std::swap(geometry[1],geometry[2]);
  efb.clear(clear,1);depthDraw.draw(efb,geometry);pixels=efb.readColor();
  bool pass=mode==0||(mode==1&&!reverse)||(mode==2&&reverse);
  if((pixels[center]==255)!=pass)std::fprintf(stderr,"Cull mismatch mode %u reverse %u red %u\n",mode,reverse,pixels[center]);assert((pixels[center]==255)==pass);++cullChecks;
 }
 bp(0,0);depthDraw.setCullMode(material);bp(0x40,0);depthDraw.setDepthMode(material);
 auto geometry=triangle(-.5f,true);for(auto& vertex:geometry)vertex.color[3]=128.f/255;
 float backdrop[4]={0,0,1,64.f/255};
 for(unsigned mask=0;mask<4;++mask){
  bp(0x41,((mask&1)?8:0)|((mask&2)?16:0));depthDraw.setBlendMode(material);efb.clear(backdrop,1);depthDraw.draw(efb,geometry);pixels=efb.readColor();
  assert(pixels[center]==((mask&1)?255:0)&&pixels[center+2]==((mask&1)?0:255)&&pixels[center+3]==((mask&2)?128:64));
 }
 bp(0x41,24|1|(4<<8)|(5<<5));depthDraw.setBlendMode(material);efb.clear(backdrop,1);depthDraw.draw(efb,geometry);pixels=efb.readColor();
 assert(std::abs(int(pixels[center])-128)<=1&&std::abs(int(pixels[center+2])-127)<=1&&std::abs(int(pixels[center+3])-96)<=1);
 bp(0x41,24|2048);depthDraw.setBlendMode(material);efb.clear(backdrop,1);depthDraw.draw(efb,geometry);pixels=efb.readColor();assert(pixels[center]==0&&pixels[center+2]==255&&pixels[center+3]==0);
 // Override stored alpha without replacing the TEV alpha used by RGB blending/tests.
 bp(0,0);bp(0x28,0);bp(0xf3,(7<<16)|(7<<19));
 bp(0xc0,(15<<12)|(10<<8)|(12<<4)|15|(1<<19));bp(0xc1,(7<<13)|(7<<10)|(7<<7)|(5<<4)|(1<<19));
 bp(0x42,256|37);
 auto overrideDraw=[&](){draw.setUntexturedMaterial(material);efb.clear(backdrop,1);draw.draw(efb,geometry);pixels=efb.readColor();};
 bp(0x41,24|1|(4<<8)|(5<<5));overrideDraw();
 assert(std::abs(int(pixels[center])-128)<=1&&std::abs(int(pixels[center+2])-127)<=1&&pixels[center+3]==37);
 bp(0x42,256|99);bool needsRebind=false;try{draw.setBlendMode(material);}catch(const std::invalid_argument&){needsRebind=true;}assert(needsRebind);bp(0x42,256|37);
 bp(0x41,8|1|(4<<8)|(5<<5));overrideDraw();assert(pixels[center+3]==64);
 bp(0x41,24|2048);overrideDraw();assert(pixels[center]==0&&pixels[center+2]==255&&pixels[center+3]==37);
 bp(0x41,24);overrideDraw();assert(pixels[center]==255&&pixels[center+2]==0&&pixels[center+3]==37);
 bp(0xf3,100|(4<<16)|(7<<19));overrideDraw();assert(pixels[center]==255&&pixels[center+3]==37);
 bp(0xf3,150|(4<<16)|(7<<19));overrideDraw();assert(pixels[center]==0&&pixels[center+3]==64);
 bp(0xf3,(7<<16)|(7<<19));bp(0x42,0);bp(0x41,24|1|(4<<8)|(5<<5));overrideDraw();assert(std::abs(int(pixels[center+3])-96)<=1);
 std::puts("PASS destination alpha: original RGB factors, write mask, subtraction, replacement, alpha tests and disable");
 // Authored ambient/material state -> vertex lighting -> TEV -> GPU framebuffer.
 bp(0,0);bp(0x28,0);bp(0x40,0);bp(0x41,24);bp(0xf3,(7<<16)|(7<<19));
 bp(0xc0,(15<<12)|(10<<8)|(12<<4)|15|(1<<19));bp(0xc1,(7<<13)|(7<<10)|(7<<7)|(5<<4)|(1<<19));
 auto xf=[&](unsigned address,unsigned value){std::vector<u8> bytes{0x10,0,0,u8(address>>8),u8(address),u8(value>>24),u8(value>>16),u8(value>>8),u8(value)};material.consume(bytes.data(),bytes.size());};
 xf(0x100a,0x80808080);xf(0x100c,0x80808080);xf(0x100e,2);xf(0x1010,0);
 TProj3f projection{};for(unsigned i=0;i<4;++i)projection.mMtx[i][i]=1;
 auto input=triangle(-.5f,true);std::vector<NativeShapeVertex> original(3);
 for(unsigned i=0;i<3;++i)std::copy_n(input[i].position,3,original[i].position.begin());
 auto lit=projectModelTriangles(original,{{0,1,2}},projection,{1,1,1,1},&material,true);
 assert(lit[0].color[0]==64.f/255&&lit[0].color[3]==128.f/255);
 draw.setUntexturedMaterial(material);efb.clear(clear,1);draw.draw(efb,lit);pixels=efb.readColor();assert(pixels[center]==64&&pixels[center+1]==64&&pixels[center+2]==64&&pixels[center+3]==128);
 xf(0x100e,3);bool missingColor=false;try{projectModelTriangles(original,{{0,1,2}},projection,{1,1,1,1},&material,true);}catch(const std::invalid_argument&){missingColor=true;}assert(missingColor);
 std::puts("PASS authored model lighting through projection, TEV rasterization and GPU readback");
 std::puts("PASS GX blend masks, source-alpha blending and reverse subtraction");
 std::printf("PASS GX culling: %u mode/winding cases including cull-all\n",cullChecks);
 std::printf("PASS GX depth state: %u compare cases, disabled writes and disabled comparison\n",depthChecks);
 std::puts("PASS independent projective texture coordinate channels 0, 1 and 7");
 std::puts("PASS textured TEV rasterization: slot-3 binding, projective UV division and sampled RGBA");
 std::printf("PASS GX alpha testing: %u boundary/function/boolean cases and discard depth preservation\n",alphaChecks);
 std::puts("PASS untextured TEV pixel rasterization: authored bias/raster selectors, alpha and unsupported texture rejection");
 std::puts("PASS native projected model triangles: persistent target, GX depth conversion, occlusion and far clipping");
 testDepthTexture(*context.Get(),*device.Get());
}












