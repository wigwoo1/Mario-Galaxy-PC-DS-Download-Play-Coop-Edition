#pragma once
#include "graphics.hpp"
#include <JSystem/J3DGraphBase/J3DTexture.hpp>

// Keep disk-relative texture payloads and native-endian headers in one owner.
// The owner must outlive any J3DTexture referencing its headers.
struct NativeTextureTables {
 std::vector<uint8_t> storage;
 uint16_t count=0;
 uint32_t headersOffset=0;
 NativeTextureTables(const uint8_t* data,size_t size){
  static_assert(sizeof(ResTIMG)==32);
  extent(0,20,size);
  assetCheck(!std::memcmp(data,"TEX1",4) && be32(data+4)==size,"Invalid TEX1 block");
  count=uint16_t(data[8])<<8|data[9];headersOffset=be32(data+12);
  storage.assign(data,data+size);
  if(count==0){
   assetCheck(headersOffset==0 || (headersOffset>=20 && headersOffset%4==0 && headersOffset<=size),
              "Invalid empty TEX1 header offset");
   return;
  }
  assetCheck(headersOffset>=20 && headersOffset%4==0,
             (std::string("Invalid TEX1 header offset=")+std::to_string(headersOffset)+
              " count="+std::to_string(count)+" size="+std::to_string(size)).c_str());
  extent(headersOffset,size_t(count)*32,size);
  for(unsigned i=0;i<count;++i){
   size_t base=headersOffset+i*32;const auto* raw=data+base;
   assetCheck(raw[8]<=1 && raw[16]<=1 && raw[17]<=1 && raw[18]<=1,"Invalid texture booleans");
   ResTIMG header{};std::memcpy(&header,raw,32);
   auto half=[&](unsigned o){return uint16_t(raw[o])<<8|raw[o+1];};
   header.mWidth=half(2);header.mHeight=half(4);header.mPaletteNum=half(10);
   header.mPaletteDataOffset=be32(raw+12);header.mImageDataOffset=be32(raw+28);
   auto bias=half(26);header.mLodBias=bias>=32768?int(bias)-65536:bias;
   assetCheck(header.mWidth && header.mHeight && header.mWidth<=1024 && header.mHeight<=1024 && header.mImageNum,"Invalid texture dimensions/mips");
   assetCheck(header.mWrapS<=2 && header.mWrapT<=2 && header.mMinType<=5 && header.mMagType<=1 && header.mMaxAnisotropy<=2,"Invalid texture sampler");
   size_t image=base+uint64_t(header.mImageDataOffset);
   assetCheck(image%32==0,"Unaligned TEX1 image payload");
   unsigned width=header.mWidth,height=header.mHeight;
   for(unsigned level=0;level<header.mImageNum;++level){
    auto bytes=gxEncodedSize(width,height,header.mFormat);extent(image,bytes,size);image+=bytes;
    width=std::max(1u,width/2);height=std::max(1u,height/2);
   }
   if(header.mPaletteName){assetCheck(header.mPaletteFormat<=2 && header.mPaletteNum,"Invalid texture palette");auto palette=base+uint64_t(header.mPaletteDataOffset);assetCheck(palette%32==0,"Unaligned TEX1 palette");extent(palette,size_t(header.mPaletteNum)*2,size);}
   std::memcpy(storage.data()+base,&header,32);
  }
 }
 ResTIMG* headers(){return reinterpret_cast<ResTIMG*>(storage.data()+headersOffset);}
};
