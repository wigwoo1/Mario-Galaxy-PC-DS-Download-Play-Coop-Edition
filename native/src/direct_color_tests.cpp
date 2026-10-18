#include "native_shape_vertices.hpp"
#include <cassert>
#include <cstdio>
static void cp(Native::GxCommandState& s,unsigned reg,unsigned value){unsigned char b[]={8,(unsigned char)reg,(unsigned char)(value>>24),(unsigned char)(value>>16),(unsigned char)(value>>8),(unsigned char)value};s.consume(b,6);}
int main(){
 Native::GxCommandState state;cp(state,0x50,(1<<13)|(1<<15));
 const u8 rgb565[]={0xf8,0x1f};cp(state,0x70,0);assert((state.directColor(0,0,rgb565,2)==std::array<unsigned,4>{255,0,255,255}));
 const u8 bytes[]={17,34,51,68};
 for(unsigned format:{1u,2u,5u})for(unsigned channel=0;channel<2;++channel){cp(state,0x70,format<<(14+channel*4));auto c=state.directColor(0,channel,bytes,4);assert(c[0]==17&&c[1]==34&&c[2]==51&&c[3]==(format==5?68u:255u));}
 const u8 rgba4[]={0x12,0x34};cp(state,0x70,3<<14);assert((state.directColor(0,0,rgba4,2)==std::array<unsigned,4>{17,34,51,68}));
 const u8 rgba6[]={0x04,0x20,0xc4};cp(state,0x70,4<<14);assert((state.directColor(0,0,rgba6,3)==std::array<unsigned,4>{4,8,12,16}));
 bool rejected=false;try{state.directColor(0,0,rgba6,2);}catch(const std::length_error&){rejected=true;}assert(rejected);
 cp(state,0x70,6<<14);rejected=false;try{state.directColor(0,0,bytes,4);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
  cp(state,0x50,(1<<9)|(1<<13)|(1<<15));cp(state,0x70,1|(4<<1)|(5<<14)|(3<<18));
 auto word=[](std::vector<u8>& b,u32 value){for(int shift=24;shift>=0;shift-=8)b.push_back(value>>shift);};
 std::vector<u8> xf{0x10};word(xf,11u<<16);for(float value:{1.f,0.f,0.f,0.f,0.f,1.f,0.f,0.f,0.f,0.f,1.f,0.f}){u32 bits;std::memcpy(&bits,&value,4);word(xf,bits);}state.consume(xf.data(),xf.size());
 std::vector<u8> geometry{0x90,0,3};
 for(unsigned i=0;i<3;++i){for(float value:{float(i),2.f,3.f}){u32 bits;std::memcpy(&bits,&value,4);word(geometry,bits);}geometry.insert(geometry.end(),{u8(17+i),34,51,68,0x12,0x34});}
 Native::GpuResources resources;std::vector<std::array<u32,3>> triangles;
 auto vertices=decodeShapePositions(state,geometry.data(),geometry.size(),resources,&triangles);
 assert(vertices.size()==3&&triangles.size()==1);
 for(unsigned i=0;i<3;++i){assert(vertices[i].position[0]==i&&vertices[i].hasColor[0]&&vertices[i].hasColor[1]);assert((vertices[i].colors[0]==std::array<unsigned,4>{17+i,34,51,68}));assert((vertices[i].colors[1]==std::array<unsigned,4>{17,34,51,68}));}
 std::puts("PASS all six direct packed GX color formats, both channels and invalid/truncated rejection");
}

