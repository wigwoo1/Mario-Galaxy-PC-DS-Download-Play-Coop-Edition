#pragma once
#include "native_material_header.hpp"
#include <JSystem/J3DGraphLoader/J3DMaterialFactory.hpp>
#include <cstddef>
#include <type_traits>
struct NativeMaterialInitTables {
 NativeMaterialHeader header;std::vector<J3DMaterialInitData> records;
 std::vector<GXCullMode> cullModes;std::vector<u16> textureNumbers;std::vector<u8> colorChannelCounts,texGenCounts,tevStageCounts;
 std::vector<GXColor> materialColors,ambientColors,konstColors;std::vector<GXColorS10> tevColors;
 std::vector<J3DColorChanInfo> colorChannels;std::vector<J3DTexCoordInfo> texCoords,postTexCoords;std::vector<J3DTevOrderInfo> tevOrders;std::vector<J3DTevStageInfo> tevStages;std::vector<J3DTevSwapModeInfo> swapModes;std::vector<J3DTevSwapModeTableInfo> swapTables;
 std::vector<J3DAlphaCompInfo> alphaTests;std::vector<J3DBlendInfo> blends;std::vector<J3DZModeInfo> depthModes;std::vector<u8> depthCompareLocations,ditherFlags;
 std::vector<J3DFogInfo> fog;std::vector<J3DNBTScaleInfo> nbtScales;
 std::vector<J3DTexMtxInfo> textureMatrices,postTextureMatrices;std::vector<J3DLightInfo> lights;
 std::vector<J3DIndInitData> indirect;
 NativeMaterialInitTables(const uint8_t* data,size_t size):header(data,size){
  static_assert(sizeof(J3DMaterialInitData)==0x14c);
  static_assert(offsetof(J3DMaterialInitData,mTexNoIdx)==0x84 && offsetof(J3DMaterialInitData,mFogIdx)==0x144);
  auto span=[&](size_t table){size_t start=header.offsets.at(table),end=be32(data+4);if(!start)return std::pair<size_t,size_t>{0,0};for(auto offset:header.offsets)if(offset>start && offset<end)end=offset;return std::pair<size_t,size_t>{start,end-start};};
  auto cull=span(4);for(size_t at=0;at+4<=cull.second;at+=4)cullModes.push_back(GXCullMode(be32(data+cull.first+at)));
  auto tex=span(15);for(size_t at=0;at+2<=tex.second;at+=2)textureNumbers.push_back(u16(data[tex.first+at])<<8|data[tex.first+at+1]);
  auto readBytes=[&](size_t index,std::vector<u8>& out){auto region=span(index);if(region.second)out.assign(data+region.first,data+region.first+region.second);};
  readBytes(6,colorChannelCounts);readBytes(10,texGenCounts);readBytes(19,tevStageCounts);
  auto colors=[&](size_t table,std::vector<GXColor>& out){auto region=span(table);for(size_t at=0;at+4<=region.second;at+=4){auto p=data+region.first+at;out.push_back({p[0],p[1],p[2],p[3]});}};
  colors(5,materialColors);colors(8,ambientColors);colors(18,konstColors);
  auto tev=span(17);auto signedHalf=[](const uint8_t* p)->s16{unsigned bits=unsigned(p[0])<<8|p[1];return s16(bits>=0x8000?int(bits)-0x10000:int(bits));};
  for(size_t at=0;at+8<=tev.second;at+=8){auto p=data+tev.first+at;tevColors.push_back({signedHalf(p),signedHalf(p+2),signedHalf(p+4),signedHalf(p+6)});}
  static_assert(sizeof(J3DColorChanInfo)==8 && sizeof(J3DTexCoordInfo)==4 && sizeof(J3DTevOrderInfo)==4 && sizeof(J3DTevStageInfo)==20 && sizeof(J3DTevSwapModeInfo)==4 && sizeof(J3DTevSwapModeTableInfo)==4);
  auto byteRecords=[&](size_t table,auto& out){using Record=typename std::decay_t<decltype(out)>::value_type;auto region=span(table);out.resize(region.second/sizeof(Record));if(!out.empty())std::memcpy(out.data(),data+region.first,out.size()*sizeof(Record));};
  byteRecords(7,colorChannels);byteRecords(11,texCoords);byteRecords(12,postTexCoords);byteRecords(16,tevOrders);byteRecords(20,tevStages);byteRecords(21,swapModes);byteRecords(22,swapTables);
  static_assert(sizeof(J3DAlphaCompInfo)==8 && sizeof(J3DBlendInfo)==4 && sizeof(J3DZModeInfo)==4);
  byteRecords(24,alphaTests);byteRecords(25,blends);byteRecords(26,depthModes);readBytes(27,depthCompareLocations);readBytes(28,ditherFlags);
  static_assert(sizeof(J3DFogInfo)==0x2c && sizeof(J3DNBTScaleInfo)==16);
  auto half=[](const uint8_t* p){return u16(u16(p[0])<<8|p[1]);};
  auto real=[](const uint8_t* p){u32 bits=be32(p);f32 value;std::memcpy(&value,&bits,4);return value;};
  auto fogRegion=span(23);fog.resize(fogRegion.second/sizeof(J3DFogInfo));for(size_t i=0;i<fog.size();++i){auto p=data+fogRegion.first+i*0x2c;auto& out=fog[i];std::memcpy(&out,p,sizeof(out));out.mCenter=half(p+2);out.mStartZ=real(p+4);out.mEndZ=real(p+8);out.mNearZ=real(p+12);out.mFarZ=real(p+16);for(unsigned j=0;j<10;++j)out.mFogAdjTable.r[j]=half(p+24+j*2);}
  auto nbtRegion=span(29);nbtScales.resize(nbtRegion.second/16);for(size_t i=0;i<nbtScales.size();++i){auto p=data+nbtRegion.first+i*16;auto& out=nbtScales[i];std::memcpy(&out,p,16);out.mScale={real(p+4),real(p+8),real(p+12)};}
  static_assert(sizeof(J3DTexMtxInfo)==0x64 && sizeof(J3DLightInfo)==0x34);
  auto vector=[&](const uint8_t* p){return Vec{real(p),real(p+4),real(p+8)};};
  auto matrices=[&](size_t table,std::vector<J3DTexMtxInfo>& out){auto region=span(table);out.resize(region.second/0x64);for(size_t i=0;i<out.size();++i){auto p=data+region.first+i*0x64;auto& m=out[i];std::memcpy(&m,p,sizeof(m));m.mCenter=vector(p+4);m.mSRT.mScaleX=real(p+16);m.mSRT.mScaleY=real(p+20);m.mSRT.mRotation=signedHalf(p+24);m.mSRT.mTranslationX=real(p+28);m.mSRT.mTranslationY=real(p+32);for(unsigned row=0;row<4;++row)for(unsigned col=0;col<4;++col)m.mEffectMtx[row][col]=real(p+36+(row*4+col)*4);}};
  matrices(13,textureMatrices);matrices(14,postTextureMatrices);
  auto lightRegion=span(9);lights.resize(lightRegion.second/0x34);for(size_t i=0;i<lights.size();++i){auto p=data+lightRegion.first+i*0x34;auto& light=lights[i];std::memcpy(&light,p,sizeof(light));light.mLightPosition=vector(p);light.mLightDirection=vector(p+12);light.mCosAtten=vector(p+28);light.mDistAtten=vector(p+40);}
  static_assert(sizeof(J3DIndInitData)==0x138 && sizeof(J3DIndTexMtxInfo)==0x1c);
  auto indRegion=span(3);indirect.resize(indRegion.second/0x138);for(size_t i=0;i<indirect.size();++i){auto p=data+indRegion.first+i*0x138;assetCheck(p[0]<=1 && p[1]<=4,"Invalid indirect material header");auto& out=indirect[i];std::memcpy(&out,p,sizeof(out));out.mEnabled=p[0]!=0;for(unsigned m=0;m<3;++m)for(unsigned row=0;row<2;++row)for(unsigned col=0;col<3;++col)out.mIndTexMtxInfo[m].field_0x0[row][col]=real(p+20+m*28+(row*3+col)*4);}
  if(header.remap.empty())return;
  size_t count=*std::max_element(header.remap.begin(),header.remap.end())+size_t(1);
  records.resize(count);
  for(size_t i=0;i<count;++i){
   auto src=data+header.offsets[0]+i*0x14c;auto& record=records[i];std::memcpy(&record,src,sizeof(record));
   // Preserve byte fields and unknown byte arrays; convert every declared u16 field.
   auto decode=[&](u16* values,size_t offset,size_t length){for(size_t k=0;k<length;++k)values[k]=u16(src[offset+k*2])<<8|src[offset+k*2+1];};
   decode(record.mMatColorIdx,8,2);decode(record.mColorChanIdx,12,4);decode(record.mAmbColorIdx,20,2);
   decode(record.mTexCoordIdx,0x28,8);decode(record.mTexMtxIdx,0x48,8);decode(record.mTexNoIdx,0x84,8);decode(record.mTevKColorIdx,0x94,4);
   decode(record.mTevOrderIdx,0xbc,16);decode(record.mTevColorIdx,0xdc,4);decode(record.mTevStageIdx,0xe4,16);decode(record.mTevSwapModeIdx,0x104,16);decode(record.mTevSwapModeTableIdx,0x124,4);
   auto reference=[](unsigned index,size_t count,unsigned absent){assetCheck(index==absent || index<count,"MAT3 scalar table index outside table");};
   reference(record.mCullModeIdx,cullModes.size(),255);reference(record.mColorChanNumIdx,colorChannelCounts.size(),255);reference(record.mTexGenNumIdx,texGenCounts.size(),255);reference(record.mTevStageNumIdx,tevStageCounts.size(),255);
   for(auto index:record.mTexMtxIdx)reference(index,textureMatrices.size(),65535);
   for(auto index:record.mTexNoIdx)reference(index,textureNumbers.size(),65535);
   for(auto index:record.mMatColorIdx)reference(index,materialColors.size(),65535);for(auto index:record.mAmbColorIdx)reference(index,ambientColors.size(),65535);for(auto index:record.mTevKColorIdx)reference(index,konstColors.size(),65535);for(auto index:record.mTevColorIdx)reference(index,tevColors.size(),65535);
   for(auto index:record.mColorChanIdx)reference(index,colorChannels.size(),65535);for(auto index:record.mTexCoordIdx)reference(index,texCoords.size(),65535);for(auto index:record.mTevOrderIdx)reference(index,tevOrders.size(),65535);for(auto index:record.mTevStageIdx)reference(index,tevStages.size(),65535);for(auto index:record.mTevSwapModeIdx)reference(index,swapModes.size(),65535);for(auto index:record.mTevSwapModeTableIdx)reference(index,swapTables.size(),65535);
   decode(&record.mFogIdx,0x144,1);decode(&record.mAlphaCompIdx,0x146,1);decode(&record.mBlendIdx,0x148,1);decode(&record.mNBTScaleIdx,0x14a,1);
   reference(record.mFogIdx,fog.size(),65535);reference(record.mNBTScaleIdx,nbtScales.size(),65535);reference(record.mAlphaCompIdx,alphaTests.size(),65535);reference(record.mBlendIdx,blends.size(),65535);reference(record.mZModeIdx,depthModes.size(),255);reference(record.mZCompLocIdx,depthCompareLocations.size(),255);reference(record.mDitherIdx,ditherFlags.size(),255);
  }
 }
};








