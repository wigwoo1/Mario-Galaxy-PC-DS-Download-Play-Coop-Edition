#include "native_shape_vertices.hpp"
#include <cassert>
#include <cstdio>
static void cp(Native::GxCommandState& s,unsigned reg,unsigned value){unsigned char b[]={8,(unsigned char)reg,(unsigned char)(value>>24),(unsigned char)(value>>16),(unsigned char)(value>>8),(unsigned char)value};s.consume(b,6);}
int main(){
 Native::GxCommandState state;cp(state,0x50,1<<9);cp(state,0x70,1|(3<<1)|(1<<4));
 const unsigned char signedValues[]={0xff,0xfe,0,4,0,6};auto v=state.directPosition(0,signedValues,6);assert(v[0]==-1&&v[1]==2&&v[2]==3);
 cp(state,0x70,0|(4<<1));const unsigned char floats[]={0x3f,0x80,0,0,0xc0,0,0,0};v=state.directPosition(0,floats,8);assert(v[0]==1&&v[1]==-2&&v[2]==0);
 bool rejected=false;try{state.directPosition(0,floats,7);}catch(const std::length_error&){rejected=true;}assert(rejected);
 const unsigned char invalid[]={0x7f,0x80,0,0,0,0,0,0};rejected=false;try{state.directPosition(0,invalid,8);}catch(const std::runtime_error&){rejected=true;}assert(rejected);
 cp(state,0x70,1|(0<<1)|(3<<4));const unsigned char bytes[]={8,16,24};v=state.directPosition(0,bytes,3);assert(v[0]==8&&v[1]==16&&v[2]==24);
 cp(state,0x70,1|(1u<<30)|(3<<4));v=state.directPosition(0,bytes,3);assert(v[0]==1&&v[1]==2&&v[2]==3);
  cp(state,0x70,1|(4<<1));
 auto word=[](std::vector<u8>& b,u32 value){for(int shift=24;shift>=0;shift-=8)b.push_back(value>>shift);};
 std::vector<u8> xf{0x10};word(xf,11u<<16);
 for(float value:{1.f,0.f,0.f,10.f,0.f,1.f,0.f,20.f,0.f,0.f,1.f,30.f}){u32 bits;std::memcpy(&bits,&value,4);word(xf,bits);}
 state.consume(xf.data(),xf.size());
 std::vector<u8> geometry{0x90,0,3};
 for(float value:{1.f,2.f,3.f,4.f,5.f,6.f,7.f,8.f,9.f}){u32 bits;std::memcpy(&bits,&value,4);word(geometry,bits);}
 Native::GpuResources resources;std::vector<std::array<u32,3>> triangles;
 auto vertices=decodeShapePositions(state,geometry.data(),geometry.size(),resources,&triangles);
 assert(vertices.size()==3&&triangles.size()==1&&(triangles[0]==std::array<u32,3>{0,1,2}));
 for(unsigned i=0;i<3;++i){assert(vertices[i].objectPosition[0]==1+3*i);assert(vertices[i].position[0]==11+3*i&&vertices[i].position[1]==22+3*i&&vertices[i].position[2]==33+3*i);}
 rejected=false;try{decodeShapePositions(state,geometry.data(),geometry.size()-1,resources);}catch(const std::runtime_error&){rejected=true;}assert(rejected);
 std::puts("PASS direct GX position signed/floating/byte fraction formats and rejection");
}

