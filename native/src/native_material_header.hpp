#pragma once
#include "assets.hpp"
#include <array>
// Wii MAT3 header: 30 four-byte offsets following the 12-byte scalar header.
// Keep offsets separate from native pointers; table records still need decoding.
struct NativeMaterialHeader {
 uint16_t materialCount;std::array<uint32_t,30> offsets{};std::vector<uint16_t> remap;
 explicit NativeMaterialHeader(const uint8_t* data,size_t size){
  assetCheck(data && size>=132,"Truncated MAT3 header");
  assetCheck(!std::memcmp(data,"MAT3",4),"Expected MAT3 block");
  auto length=be32(data+4);assetCheck(length>=132 && length<=size,"Invalid MAT3 length");
  materialCount=uint16_t(data[8])<<8|data[9];
  for(size_t i=0;i<offsets.size();++i){offsets[i]=be32(data+12+i*4);assetCheck(!offsets[i] || (offsets[i]>=132 && offsets[i]<length),"MAT3 table offset outside block");}  if(materialCount){
   assetCheck(offsets[0] && offsets[1],"Missing MAT3 initialization/remap table");
   auto end=[&](uint32_t start){uint32_t result=length;for(auto value:offsets)if(value>start && value<result)result=value;return result;};
   extent(offsets[1],size_t(materialCount)*2,end(offsets[1]));
   auto initEnd=end(offsets[0]);size_t records=(initEnd-offsets[0])/0x14c;
   assetCheck(records>0,"Truncated MAT3 initialization records");
   remap.reserve(materialCount);
   for(size_t i=0;i<materialCount;++i){size_t at=offsets[1]+i*2;uint16_t index=uint16_t(data[at])<<8|data[at+1];assetCheck(index<records,"MAT3 remap index outside initialization table");remap.push_back(index);}
  }
 }
};

