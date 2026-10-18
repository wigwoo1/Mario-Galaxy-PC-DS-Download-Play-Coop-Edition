#pragma once
#include "native_name_table.hpp"
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
#include <memory>
#include <JSystem/JKernel/JKRArchive.hpp>
struct NativeTexturePatternAnimation final : J3DAnmTexPattern {
 std::vector<J3DAnmTexPatternFullTable> tables;
 std::vector<u16> indices,ids;
 std::unique_ptr<NativeNameTable> names;
 NativeTexturePatternAnimation(const void* data,size_t size){
  assetCheck(data&&size>=32,"Missing BTP");
  auto* p=static_cast<const u8*>(data);std::vector<u8> copy(p,p+size);BinaryView view(copy);
  assetCheck(!std::memcmp(p,"J3D1btp1",8),"Invalid BTP magic");
  size_t declared=view.u32(8);assetCheck(declared>=32&&declared<=size,"Invalid BTP declared size");
  size_t at=32;bool found=false;
  for(unsigned i=0;i<view.u32(12);i++){
   extent(at,8,declared);size_t length=view.u32(at+4);assetCheck(length>=8,"Invalid BTP block");extent(at,length,declared);
   if(!std::memcmp(p+at,"TPT1",4)){
    assetCheck(!found&&length>=32,"Invalid BTP pattern block");found=true;auto block=view.sub(at,length);
    mAttribute=block.u8(8);mFrameMax=s16(block.u16(10));mFrame=0;mUpdateMaterialNum=block.u16(12);field_0x14=block.u16(14);
    size_t t=block.u32(16),v=block.u32(20),id=block.u32(24),name=block.u32(28);
    assetCheck(t>=32&&v>=32&&id>=32&&name>=32,"BTP offsets overlap header");
    extent(t,size_t(mUpdateMaterialNum)*8,length);extent(v,size_t(field_0x14)*2,length);extent(id,size_t(mUpdateMaterialNum)*2,length);extent(name,4,length);
    for(unsigned j=0;j<field_0x14;j++)indices.push_back(block.u16(v+j*2));
    for(unsigned j=0;j<mUpdateMaterialNum;j++){
     size_t o=t+j*8;u16 count=block.u16(o),offset=block.u16(o+2);
     assetCheck(count&&offset<=indices.size()&&count<=indices.size()-offset,"Invalid BTP track");
     assetCheck(block.u8(o+4)<8,"BTP texture slot exceeds material animator capacity");
     tables.push_back({count,offset,block.u8(o+4),block.u16(o+6)});ids.push_back(block.u16(id+j*2));
    }
    names=std::make_unique<NativeNameTable>(block.sub(name,length-name));
    assetCheck(names->storage[0]>=mUpdateMaterialNum,"BTP material name count too small");
   }
   at+=length;
  }
  assetCheck(found,"Missing TPT1");mTextureIndex=indices.data();mAnmTable=tables.data();mUpdateMaterialID=ids.data();mUpdateMaterialName.setResource(names->resource());
 }
 NativeTexturePatternAnimation(const NativeTexturePatternAnimation&)=delete;
 NativeTexturePatternAnimation& operator=(const NativeTexturePatternAnimation&)=delete;
};
inline J3DAnmBase* loadNativeArchiveTexturePattern(const JKRArchive& archive,const void* data){
 if(!data)return nullptr;
 s32 size=archive.getResSize(data);assetCheck(size>=32,"BTP resource not owned by archive or truncated");
 return new NativeTexturePatternAnimation(data,size_t(size));
}

