#pragma once
#include "gx_command_texture.hpp"
#include <cassert>
inline void testDepthTexture(ID3D11DeviceContext& context,ID3D11Device& device){
 Native::D3dEfb efb(context,32,32);Native::D3dModelDraw draw(context);Native::GxCommandState state;
 auto bp=[&](unsigned reg,unsigned value){u32 word=(reg<<24)|value;u8 b[]={0x61,u8(word>>24),u8(word>>16),u8(word>>8),u8(word)};state.consume(b,5);};
 bp(0,1);bp(0x41,24);bp(0x40,31);bp(0xf3,(7<<16)|(7<<19));bp(0xf6,4);bp(0xf7,14);bp(0x28,64);
 bp(0xc0,(15<<12)|(8<<8)|(12<<4)|15|(1<<19));bp(0xc1,(7<<13)|(7<<10)|(7<<7)|(4<<4)|(1<<19));
 auto geometry=[](float z){return std::vector<Native::ProjectedVertex>{{{-.8f,-.8f,z,1},{1,0,0,1}},{{0,.8f,z,1},{1,0,0,1}},{{.8f,-.8f,z,1},{1,0,0,1}}};};
 float black[]={0,0,0,1};std::array<ID3D11ShaderResourceView*,8> views{};std::array<ID3D11SamplerState*,8> samplers{};
 // Original MainLoopFramework clearZTexData words: A=0, RGB=255 on GX.
 u32 authoredWords[16]={0x00ff00ff,0x00ff00ff,0x00ff00ff,0x00ff00ff,0x00ff00ff,0x00ff00ff,0x00ff00ff,0x00ff00ff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff};
 Native::GpuResources storage;auto address=storage.addNativeWords(authoredWords,sizeof(authoredWords));
 auto* bytes=storage.resolve(address,sizeof(authoredWords));std::vector<u8> encodedBytes(bytes,bytes+sizeof(authoredWords));
 auto maxDepth=decodeGxTexture(BinaryView(encodedBytes),4,4,6,0,BinaryView(encodedBytes,0,0));
 for(auto pixel:maxDepth.pixels)assert(pixel.r==255&&pixel.g==255&&pixel.b==255&&pixel.a==0);
 assert(authoredWords[0]==0x00ff00ff);NativeModelTexture maxTexture(device,maxDepth);views[0]=maxTexture.view.Get();samplers[0]=maxTexture.sampler.Get();
 bp(0xf5,2|(2<<2));draw.setMaterial(state,views,samplers);efb.clear(black,1);draw.draw(efb,geometry(-1));
 auto depth=efb.readDepth();assert(std::fabs(depth[16*32+16]-16777215.f/16777216.f)<1e-7f);
 Native::D3dModelDraw world(context);world.draw(efb,geometry(-.5f));auto color=efb.readColor();assert(color[(16*32+16)*4]==255&&color[(16*32+16)*4+1]==0);
 std::puts("PASS original clear pattern: Z24 texture replaces near geometry depth with far depth, allowing a later world triangle");
 TextureImage source{1,1,6,{{17,34,51,68}}};NativeModelTexture texture(device,source);views[0]=texture.view.Get();samplers[0]=texture.sampler.Get();
 const unsigned encoded[]={68,17+68*256,17*65536+34*256+51};
 for(unsigned type=0;type<3;++type)for(unsigned operation:{1u,2u}){
  bp(0xf4,0xffff00);bp(0xf5,type|(operation<<2));draw.setMaterial(state,views,samplers);efb.clear(black,1);draw.draw(efb,geometry(-.75f));depth=efb.readDepth();
  unsigned expected=(encoded[type]+0xffff00+(operation==1?0x400000:0))&0xffffff;
  assert(std::fabs(depth[16*32+16]-float(expected)/16777216.f)<1e-7f);
 }
 bp(0xf4,0);bp(0xf5,2|(2<<2));bp(0x43,64);draw.setMaterial(state,views,samplers);efb.clear(black,1);draw.draw(efb,geometry(-.75f));assert(std::fabs(efb.readDepth()[16*32+16]-.25f)<1e-7f);
 bp(0x43,0);bp(0xf5,0);draw.setMaterial(state,views,samplers);efb.clear(black,1);draw.draw(efb,geometry(-.75f));assert(std::fabs(efb.readDepth()[16*32+16]-.25f)<1e-7f);
 bp(0xf5,2|(2<<2));bp(0x42,256|37);draw.setMaterial(state,views,samplers);efb.clear(black,1);draw.draw(efb,geometry(-.75f));color=efb.readColor();assert(color[(16*32+16)*4+3]==37);assert(std::fabs(efb.readDepth()[16*32+16]-float(encoded[2])/16777216.f)<1e-7f);
 // A disabled final TEV stage leaves the most recent raw texture available.
 bp(0,(1<<10)|1);bp(0x28,64|(7<<19));bp(0xc2,state.bp(0xc0));bp(0xc3,state.bp(0xc1));draw.setMaterial(state,views,samplers);efb.clear(black,1);draw.draw(efb,geometry(-.75f));assert(std::fabs(efb.readDepth()[16*32+16]-float(encoded[2])/16777216.f)<1e-7f);
 bp(0,0);draw.setMaterial(state,views,samplers);efb.clear(black,1);draw.draw(efb,geometry(-.75f));assert(efb.readDepth()[16*32+16]==0);
 std::puts("PASS Z-texture U8/U16/U24 channel packing, add/replace, bias/wrap, disabled/early depth, destination alpha and last raw sampled texture");
}
