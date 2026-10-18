#include "native_shape_vertices.hpp"
#include <cassert>
#include <cstdio>
static void cp(Native::GxCommandState& s,unsigned reg,unsigned value){unsigned char b[]={8,(unsigned char)reg,(unsigned char)(value>>24),(unsigned char)(value>>16),(unsigned char)(value>>8),(unsigned char)value};s.consume(b,6);}
int main(){
 Native::GxCommandState state;cp(state,0x50,1<<11);
 const u8 n8[]={0xc0,0x20,0};cp(state,0x70,1<<10);assert((state.directNormal(0,n8,3)==std::array<float,3>{-1,.5f,0}));
 const u8 n16[]={0xc0,0,0x20,0,0,0};cp(state,0x70,3<<10);assert((state.directNormal(0,n16,6)==std::array<float,3>{-1,.5f,0}));
 const u8 nf[]={0x3f,0x80,0,0,0xc0,0,0,0,0x40,0x40,0,0};cp(state,0x70,4<<10);assert((state.directNormal(0,nf,12)==std::array<float,3>{1,-2,3}));
 bool rejected=false;try{state.directNormal(0,nf,11);}catch(const std::length_error&){rejected=true;}assert(rejected);
 cp(state,0x70,0);rejected=false;try{state.directNormal(0,n8,3);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
 cp(state,0x50,(1<<9)|(1<<11));cp(state,0x70,1|(4<<1)|(1<<10));
 auto word=[](std::vector<u8>& b,u32 v){for(int shift=24;shift>=0;shift-=8)b.push_back(v>>shift);};
 auto xf=[&](unsigned address,std::initializer_list<float> values){std::vector<u8> b{0x10};word(b,((values.size()-1)<<16)|address);for(float v:values){u32 bits;std::memcpy(&bits,&v,4);word(b,bits);}state.consume(b.data(),b.size());};
 xf(0,{1,0,0,0,0,1,0,0,0,0,1,0});xf(0x400,{0,1,0,1,0,0,0,0,1});
 std::vector<u8> geometry{0x90,0,3};
 for(unsigned i=0;i<3;++i){for(float v:{float(i),2.f,3.f}){u32 bits;std::memcpy(&bits,&v,4);word(geometry,bits);}geometry.insert(geometry.end(),{0xc0,0x20,0});}
 Native::GpuResources resources;auto vertices=decodeShapePositions(state,geometry.data(),geometry.size(),resources);
 assert(vertices.size()==3);for(unsigned i=0;i<3;++i){assert(vertices[i].position[0]==i&&vertices[i].hasNormal);assert((vertices[i].transformedNormal==std::array<float,3>{.5f,-1,0}));}
 std::puts("PASS direct signed8/signed16/float normals, fixed fractions and full triangle normal transform");
}
