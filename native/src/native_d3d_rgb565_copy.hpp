#pragma once
#include "native_rgb565_copy.hpp"
#include "native_alpha8_copy.hpp"
#include <Native/D3dEfb.hpp>
#include <Native/D3dEfbRegionClear.hpp>
class NativeD3dRgb565Copy {
 Native::D3dEfb& efb;Native::GpuResources& resources;ID3D11DeviceContext& context;
public:
 NativeD3dRgb565Copy(Native::D3dEfb& target,Native::GpuResources& storage,ID3D11DeviceContext& gpu):efb(target),resources(storage),context(gpu){}
 void operator()(const Native::GxCommandState::CopyOperation& operation)const{

  const auto command=operation.registers[0x52];
  const unsigned format=((command>>4)&7)|(((command>>3)&1)<<3);
  const bool alphaOnly=format==7;
  const auto filter0=operation.registers[0x53],filter1=operation.registers[0x54];
  // GX's vertical copy filter affects RGB only; A8 preserves current-row alpha.
  if(!alphaOnly&&(filter0||filter1)){
   unsigned coefficients[7];
   for(unsigned i=0;i<7;++i)coefficients[i]=((i<4?filter0:filter1)>>((i<4?i:i-4)*6))&63;
   // Original GXSetCopyFilter(false) encodes 0,0,21,22,21,0,0.
   // Single-sample EFB identity filters use only the current row and total weight 64.
   if(coefficients[0]||coefficients[1]||coefficients[5]||coefficients[6]||coefficients[2]+coefficients[3]+coefficients[4]!=64)
    throw std::invalid_argument("Vertical EFB copy filtering requires resampling integration");
  }
  auto rgba=efb.readColor();
  TextureImage image{efb.getWidth(),efb.getHeight(),6,std::vector<Pixel>(std::size_t(efb.getWidth())*efb.getHeight())};
  for(size_t i=0;i<image.pixels.size();++i)image.pixels[i]={rgba[i*4],rgba[i*4+1],rgba[i*4+2],rgba[i*4+3]};
  if(alphaOnly)writeAlpha8Copy(image,operation,resources);
  else writeRgb565Copy(image,operation,resources);
  if(operation.clear()){
   auto ar=operation.registers[0x4f],gb=operation.registers[0x50];
   float rgba[]={float(ar&255)/255,float((gb>>8)&255)/255,float(gb&255)/255,float((ar>>8)&255)/255};
   float depth=float(operation.registers[0x51]&0xffffff)/16777216.f;
   Native::clearD3dEfbRegion(context,efb,operation.left(),operation.top(),operation.width(),operation.height(),rgba,depth);
  }
 }
};


