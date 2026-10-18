#pragma once
#include "native_name_table.hpp"
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
#include <JSystem/JKernel/JKRArchive.hpp>
#include <cmath>
#include <memory>
struct NativeTextureSrtAnimation final:J3DAnmTextureSRTKey {
 struct Tracks {
  std::vector<J3DAnmTransformKeyTable> tables;
  std::vector<f32> scale,translation;
  std::vector<s16> rotation;
  std::vector<u16> materials;
  std::vector<u8> slots;
  std::vector<Vec> centers;
  std::unique_ptr<NativeNameTable> names;
  void read(BinaryView block,size_t counts,size_t offsets){
   const unsigned axes=block.u16(counts),sc=block.u16(counts+2),rc=block.u16(counts+4),tc=block.u16(counts+6);
   assetCheck(axes%3==0,"BTK axis count is not a material triplet");
   const size_t t=block.u32(offsets),id=block.u32(offsets+4),name=block.u32(offsets+8),slot=block.u32(offsets+12),center=block.u32(offsets+16);
   auto range=[&](size_t at,size_t bytes){if(bytes)assetCheck(at>=96,"BTK data overlaps block header");extent(at,bytes,block.size());};
   auto floats=[&](size_t at,unsigned count,std::vector<f32>& values){range(at,size_t(count)*4);for(unsigned i=0;i<count;++i){float v=block.f32(at+i*4);assetCheck(std::isfinite(v),"Nonfinite BTK value");values.push_back(v);}};
   floats(block.u32(offsets+20),sc,scale);floats(block.u32(offsets+28),tc,translation);
   const size_t rot=block.u32(offsets+24);range(rot,size_t(rc)*2);for(unsigned i=0;i<rc;++i)rotation.push_back(s16(block.u16(rot+i*2)));
   range(t,size_t(axes)*18);tables.resize(axes);
   for(unsigned i=0;i<axes;++i){
    J3DAnmKeyTableBase* tracks[]={&tables[i].mScaleInfo,&tables[i].mRotationInfo,&tables[i].mTranslateInfo};
    const size_t sizes[]={scale.size(),rotation.size(),translation.size()};
    for(unsigned kind=0;kind<3;++kind){size_t at=t+i*18+kind*6;auto& track=*tracks[kind];track={block.u16(at),block.u16(at+2),block.u16(at+4)};
     if(!track.mMaxFrame)continue;
     assetCheck(track.mType<=1,"Invalid BTK tangent mode");
     const size_t stride=track.mType?4:3,used=track.mMaxFrame==1?1:size_t(track.mMaxFrame)*stride;
     assetCheck(track.mOffset<=sizes[kind]&&used<=sizes[kind]-track.mOffset,"BTK track exceeds value table");
     if(track.mMaxFrame>1)for(unsigned key=1;key<track.mMaxFrame;++key){
      size_t a=track.mOffset+(key-1)*stride,b=a+stride;
      const float first=kind==0?scale[a]:kind==1?float(rotation[a]):translation[a];
      const float next=kind==0?scale[b]:kind==1?float(rotation[b]):translation[b];
      assetCheck(next>=first,"Unordered BTK key times");
     }
    }
   }
   const unsigned count=axes/3;range(id,size_t(count)*2);range(slot,count);range(center,size_t(count)*12);
   for(unsigned i=0;i<count;++i){materials.push_back(block.u16(id+i*2));auto matrix=block.u8(slot+i);assetCheck(matrix<8,"BTK matrix slot outside material animator");slots.push_back(matrix);
    Vec c{block.f32(center+i*12),block.f32(center+i*12+4),block.f32(center+i*12+8)};assetCheck(std::isfinite(c.x)&&std::isfinite(c.y)&&std::isfinite(c.z),"Nonfinite BTK center");centers.push_back(c);}
   if(name){assetCheck(name>=96,"BTK names overlap block header");extent(name,4,block.size());names=std::make_unique<NativeNameTable>(block.sub(name,block.size()-name));assetCheck(names->storage[0]>=count,"BTK material names missing");}
   else assetCheck(count==0,"Missing BTK material name table");
  }
 } primary,post;
 NativeTextureSrtAnimation(const void* data,size_t size){
  assetCheck(data&&size>=32,"Missing BTK header");const auto* bytes=static_cast<const u8*>(data);std::vector<u8> copy(bytes,bytes+size);BinaryView view(copy);
  assetCheck(!std::memcmp(bytes,"J3D1btk1",8),"Invalid BTK magic");const size_t declared=view.u32(8);assetCheck(declared>=32&&declared<=size,"Invalid BTK declared extent");
  size_t at=32;bool found=false;
  for(unsigned i=0;i<view.u32(12);++i){extent(at,8,declared);const size_t advertised=view.u32(at+4);assetCheck(advertised>=8,"Invalid BTK block length");size_t length=advertised;
   // Some retail exports count four omitted trailing padding bytes. Only the
   // final block may do this; every referenced table still fits actual bytes.
   if(length>declared-at){assetCheck(i+1==view.u32(12)&&length-(declared-at)==4,"BTK block exceeds file extent");length=declared-at;}
   extent(at,length,declared);
   if(view.tag(at)=="TTK1"){
    assetCheck(!found&&length>=96,"Invalid or duplicate BTK block");found=true;auto block=view.sub(at,length);
    mAttribute=block.u8(8);mDecShift=block.u8(9);mFrameMax=s16(block.u16(10));mFrame=0;
    assetCheck(mAttribute<=4&&mDecShift<=15&&mFrameMax>=0,"Invalid BTK playback header");
    primary.read(block,12,20);post.read(block,52,60);
    mTrackNum=u16(primary.tables.size());mScaleNum=u16(primary.scale.size());mRotNum=u16(primary.rotation.size());mTransNum=u16(primary.translation.size());
    mAnmTable=primary.tables.data();mScaleData=primary.scale.data();mRotData=primary.rotation.data();mTransData=primary.translation.data();
    mUpdateMaterialID=primary.materials.data();mUpdateTexMtxID=primary.slots.data();mSRTCenter=primary.centers.data();if(primary.names)mUpdateMaterialName.setResource(primary.names->resource());
    field_0x4a=u16(post.tables.size());field_0x44=u16(post.scale.size());field_0x46=u16(post.rotation.size());field_0x48=u16(post.translation.size());
    field_0x58=post.tables.data();field_0x4c=post.scale.data();field_0x50=post.rotation.data();field_0x54=post.translation.data();
    mPostUpdateMaterialID=post.materials.data();mPostUpdateTexMtxID=post.slots.data();mPostSRTCenter=post.centers.data();if(post.names)mPostUpdateMaterialName.setResource(post.names->resource());
    mTexMtxCalcType=block.u32(92);if(mTexMtxCalcType>1)mTexMtxCalcType=0;
   }at+=length;
  }assetCheck(found,"Missing BTK texture transform block");
 }
 NativeTextureSrtAnimation(const NativeTextureSrtAnimation&)=delete;
 NativeTextureSrtAnimation& operator=(const NativeTextureSrtAnimation&)=delete;
};
inline J3DAnmBase* loadNativeArchiveTextureSrt(const JKRArchive& archive,const void* data){
 if(!data)return nullptr;const s32 size=archive.getResSize(data);assetCheck(size>=32,"BTK resource is foreign or truncated");return new NativeTextureSrtAnimation(data,size_t(size));
}
