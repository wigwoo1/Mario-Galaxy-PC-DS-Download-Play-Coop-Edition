#include "native_shape_vertices.hpp"
#include <cassert>
#include <cstdio>
int main(){
 Native::GxCommandState state;Native::GpuResources resources;
 auto word=[](std::vector<u8>& bytes,u32 v){for(int shift=24;shift>=0;shift-=8)bytes.push_back(v>>shift);};
 auto cp=[&](u8 reg,u32 v){std::vector<u8> b{8,reg};word(b,v);state.consume(b.data(),b.size());};
 std::vector<u8> positions;for(float v:{1.f,2.f,3.f}){u32 bits;std::memcpy(&bits,&v,4);word(positions,bits);}
 auto address=resources.add(positions.data(),positions.size());
 cp(0x50,(2<<9)|2);cp(0x70,1|(4<<1));cp(0xa0,address);cp(0xb0,12);
 cp(0x30,(30<<6)|(33<<12)|(36<<18)|(39<<24));cp(0x40,42|(45<<6)|(48<<12)|(51<<18));
 std::vector<u8> xf{0x10};word(xf,(11<<16));for(float v:{1.f,0.f,0.f,10.f,0.f,1.f,0.f,20.f,0.f,0.f,1.f,30.f}){u32 bits;std::memcpy(&bits,&v,4);word(xf,bits);}state.consume(xf.data(),xf.size());
 std::vector<u8> geometry{0x90,0,3,60,0,57,0,54,0};
 auto vertices=decodeShapePositions(state,geometry.data(),geometry.size(),resources);
 assert(vertices.size()==3);
 for(unsigned i=0;i<3;++i){auto& v=vertices[i];assert((v.objectPosition==std::array<float,3>{1,2,3}));assert((v.position==std::array<float,3>{11,22,33}));assert(v.textureMatrices[0]==60-i*3);assert(v.textureMatrices[1]==33&&v.textureMatrices[4]==42&&v.textureMatrices[7]==51);}
 cp(0x50,2<<9);geometry={0x90,0,3,0,0,0};vertices=decodeShapePositions(state,geometry.data(),geometry.size(),resources);assert(vertices[0].textureMatrices[0]==30);
 std::puts("PASS decoded texgen inputs: object/view positions, direct texture indices and CP defaults");
}
