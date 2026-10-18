#include "native_model_texgen.hpp"
#include <cassert>
#include <cstdio>
static void cp(Native::GxCommandState& s,unsigned reg,unsigned value){unsigned char b[]={8,(unsigned char)reg,(unsigned char)(value>>24),(unsigned char)(value>>16),(unsigned char)(value>>8),(unsigned char)value};s.consume(b,6);}
int main(){
 Native::GxCommandState state;cp(state,0x50,1<<9);cp(state,0x60,0x5555);
 cp(state,0x70,1|(4<<1)|(9<<21));cp(state,0x80,9|(9<<9)|(9<<18)|(9u<<27));cp(state,0x90,(9<<5)|(9<<14)|(9<<23));
 auto word=[](std::vector<u8>& b,u32 v){for(int shift=24;shift>=0;shift-=8)b.push_back(v>>shift);};
 auto fl=[&](std::vector<u8>& b,float v){u32 bits;std::memcpy(&bits,&v,4);word(b,bits);};
 std::vector<u8> xf{0x10};word(xf,11u<<16);for(float v:{1.f,0.f,0.f,0.f,0.f,1.f,0.f,0.f,0.f,0.f,1.f,0.f})fl(xf,v);state.consume(xf.data(),xf.size());
 std::vector<u8> texgen{0x10};word(texgen,(7u<<16)|0x1040);for(unsigned c=0;c<8;++c)word(texgen,(5+c)<<7);state.consume(texgen.data(),texgen.size());
 std::vector<u8> geometry{0x90,0,3};
 for(unsigned i=0;i<3;++i){fl(geometry,float(i));fl(geometry,2);fl(geometry,3);for(unsigned c=0;c<8;++c){fl(geometry,float(c+i));fl(geometry,.5f+c);}}
 Native::GpuResources resources;std::vector<std::array<u32,3>> triangles;auto vertices=decodeShapePositions(state,geometry.data(),geometry.size(),resources,&triangles);
 assert(vertices.size()==3&&triangles.size()==1);
 for(unsigned i=0;i<3;++i){vertices[i].textureMatrices.fill(60);for(unsigned c=0;c<8;++c){assert(vertices[i].hasTexCoords[c]&&vertices[i].texCoords[c][0]==c+i&&vertices[i].texCoords[c][1]==.5f+c);auto uv=generateModelTexCoord(state,vertices[i],c);assert(uv[0]==c+i&&uv[1]==.5f+c&&uv[2]==1);}}
 // Channel 4 fraction lives in VAT C, while count/type live in VAT B.
 cp(state,0x80,7u<<27);cp(state,0x90,1);const u8 fixed[]={0xff,0xfe,0,4};
 auto v=state.directTexCoord(0,4,fixed,4);assert(v[0]==-1&&v[1]==2);
 bool rejected=false;try{state.directTexCoord(0,4,fixed,3);}catch(const std::length_error&){rejected=true;}assert(rejected);
 std::puts("PASS eight direct UV channels through triangle/texgen and split-register signed fixed-point format");
}
