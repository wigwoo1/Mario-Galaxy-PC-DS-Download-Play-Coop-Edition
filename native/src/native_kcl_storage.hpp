#pragma once
#include <Game/Map/KCollision.hpp>
#include "native_kcl_octree.hpp"
#include <cstring>
#include <cmath>
#include <algorithm>
// Native allocations retain original vector/prism types; no disk pointer overlay.
struct NativeKclStorage {
 std::vector<TVec3f> positions,normals;
 std::vector<KC_PrismData> prisms; // Index zero reserved, matching original server.
 float thickness;TVec3f minimum;
 std::uint32_t masks[3];int widthShift,xShift,xyShift;
 NativeKclOctree octree;
 explicit NativeKclStorage(const std::vector<std::uint8_t>& data):octree(decodeOctree(data)){
  auto word=[&](std::size_t p){return read32(data,p);};
  auto number=[&](std::size_t p){auto bits=word(p);float f;std::memcpy(&f,&bits,4);if(!std::isfinite(f))throw std::invalid_argument("Non-finite KCL value");return f;};
  auto half=[&](std::size_t p){if(p>data.size()||data.size()-p<2)throw std::out_of_range("KCL halfword");return std::uint16_t((std::uint16_t(data[p])<<8)|data[p+1]);};
  std::size_t pos=word(0),norm=word(4),prism=word(8),tree=word(12);
  if(pos<56||pos>norm||norm>prism+16||prism+16>tree||tree>=data.size()||(tree-prism)%16)throw std::invalid_argument("KCL section offsets");
  thickness=number(16);if(thickness<0)throw std::invalid_argument("Negative KCL thickness");minimum.set(number(20),number(24),number(28));
  for(int i=0;i<3;++i)masks[i]=word(32+i*4);widthShift=std::int32_t(word(44));xShift=std::int32_t(word(48));xyShift=std::int32_t(word(52));
  for(std::size_t i=0;i<(norm-pos)/12;++i)positions.emplace_back(number(pos+i*12),number(pos+i*12+4),number(pos+i*12+8));
  for(std::size_t i=0;i<(prism+16-norm)/12;++i)normals.emplace_back(number(norm+i*12),number(norm+i*12+4),number(norm+i*12+8));
  prisms.resize((tree-prism)/16);prisms[0]={};
  for(std::size_t i=1;i<prisms.size();++i){auto p=prism+i*16;auto& r=prisms[i];r.mHeight=number(p);r.mPositionIndex=half(p+4);r.mNormalIndex=half(p+6);for(int j=0;j<3;++j)r.mEdgeIndices[j]=half(p+8+j*2);r.mAttribute=half(p+14);
   if(r.mPositionIndex>=positions.size()||r.mNormalIndex>=normals.size())throw std::out_of_range("KCL prism vector index");for(auto edge:r.mEdgeIndices)if(edge>=normals.size())throw std::out_of_range("KCL prism edge index");
  }
 }
 std::size_t triangleCount()const{return prisms.size()-1;}
 // DynamicCollisionObj updates fixed topology in place, preserving pointers held
 // by original collision queries. Decode and validate before mutating live data.
 void updateDynamic(const std::vector<std::uint8_t>& bytes) {
  NativeKclStorage next(bytes);
  if(xShift!=-1 || xyShift!=-1 || next.xShift!=-1 || next.xyShift!=-1 ||
     positions.size()!=next.positions.size() || normals.size()!=next.normals.size() || prisms.size()!=next.prisms.size())
   throw std::invalid_argument("Dynamic KCL topology changed");
  auto oldLeaf=octree.lookup(0,0,0,widthShift,xShift,xyShift,triangleCount());
  auto newLeaf=next.octree.lookup(0,0,0,next.widthShift,next.xShift,next.xyShift,next.triangleCount());
  if(oldLeaf.prisms!=newLeaf.prisms)throw std::invalid_argument("Dynamic KCL leaf changed");
  std::copy(next.positions.begin(),next.positions.end(),positions.begin());
  std::copy(next.normals.begin(),next.normals.end(),normals.begin());
  std::copy(next.prisms.begin(),next.prisms.end(),prisms.begin());
  thickness=next.thickness;minimum=next.minimum;
  for(int i=0;i<3;++i)masks[i]=next.masks[i];
  widthShift=next.widthShift;
 }
private:
 static std::uint32_t read32(const std::vector<std::uint8_t>& d,std::size_t p){if(p>d.size()||d.size()-p<4)throw std::out_of_range("KCL word");return (std::uint32_t(d[p])<<24)|(std::uint32_t(d[p+1])<<16)|(std::uint32_t(d[p+2])<<8)|d[p+3];}
 static std::vector<std::uint8_t> decodeOctree(const std::vector<std::uint8_t>& d){if(d.size()<56)throw std::out_of_range("KCL header");auto offset=read32(d,12);if(offset>=d.size())throw std::out_of_range("KCL octree offset");return {d.begin()+offset,d.end()};}
};
