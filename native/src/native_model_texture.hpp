#pragma once
#include "graphics.hpp"
#include <Native/D3dEfb.hpp>
#include <cstdio>
// Owns all decoded mip levels and the corresponding ordinary GX sampler.
// GX bias clamp, edge LOD and anisotropy are not represented by TextureImage yet.
class NativeModelTexture {
public:
 NativeModelTexture(ID3D11Device& device,const TextureImage& source){
  if(!source.width||!source.height||source.width>8192||source.height>8192||source.wrapS>2||source.wrapT>2||source.minFilter>5||source.magFilter>1||source.minLod>source.maxLod)
   throw std::invalid_argument("Invalid model texture");
  std::vector<D3D11_SUBRESOURCE_DATA> levels;
  auto append=[&](unsigned w,unsigned h,const std::vector<Pixel>& pixels){
   if(pixels.size()!=size_t(w)*h)throw std::invalid_argument("Invalid model mip pixels");
   levels.push_back({pixels.data(),w*4,0});
  };
  append(source.width,source.height,source.pixels);
  unsigned w=source.width,h=source.height;
  for(const auto& level:source.mipmaps){
   if(w==1&&h==1)throw std::invalid_argument("Excess model mip levels");
   w=std::max(1u,w/2);h=std::max(1u,h/2);
   if(level.width!=w||level.height!=h)throw std::invalid_argument("Invalid model mip dimensions");
   append(w,h,level.pixels);
  }
  D3D11_TEXTURE2D_DESC desc{};desc.Width=source.width;desc.Height=source.height;desc.MipLevels=UINT(levels.size());desc.ArraySize=1;desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;desc.SampleDesc.Count=1;desc.Usage=D3D11_USAGE_IMMUTABLE;desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
  check(device.CreateTexture2D(&desc,levels.data(),&texture),"CreateTexture2D",device,source,desc);
  check(device.CreateShaderResourceView(texture.Get(),nullptr,&view),"CreateShaderResourceView",device,source,desc);
  auto address=[](unsigned v){return v==0?D3D11_TEXTURE_ADDRESS_CLAMP:v==1?D3D11_TEXTURE_ADDRESS_WRAP:D3D11_TEXTURE_ADDRESS_MIRROR;};
  // GX enum: nearest, linear, nearest-mip-nearest, linear-mip-nearest,
  // nearest-mip-linear, linear-mip-linear.
  unsigned minLinear=source.minFilter==1||source.minFilter==3||source.minFilter==5;
  unsigned mipLinear=source.minFilter>=4;
  D3D11_SAMPLER_DESC samplerDesc{};samplerDesc.Filter=D3D11_FILTER((minLinear?0x10:0)|(source.magFilter?4:0)|(mipLinear?1:0));
  samplerDesc.AddressU=address(source.wrapS);samplerDesc.AddressV=address(source.wrapT);samplerDesc.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.MipLODBias=source.commandLod?source.commandLodBias:source.lodBias/100.f;
  samplerDesc.MinLOD=source.minFilter<2?0:std::max(0.f,source.commandLod?source.commandMinLod:source.minLod/8.f);
  samplerDesc.MaxLOD=source.minFilter<2?0:std::max(0.f,source.commandLod?source.commandMaxLod:source.maxLod/8.f);
  check(device.CreateSamplerState(&samplerDesc,&sampler),"CreateSamplerState",device,source,desc,&samplerDesc);
 }
 Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
 Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> view;
 Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
private:
 static void check(HRESULT h,const char* stage,ID3D11Device& device,const TextureImage& source,const D3D11_TEXTURE2D_DESC& desc,const D3D11_SAMPLER_DESC* samplerState=nullptr){
  if(SUCCEEDED(h))return;
  const HRESULT removed=device.GetDeviceRemovedReason();
  char message[640];
  const int used=std::snprintf(message,sizeof(message),"Model texture D3D upload failure: stage=%s HRESULT=0x%08lX deviceReason=0x%08lX size=%ux%u levels=%u featureLevel=0x%lX gxFormat=%u minFilter=%u magFilter=%u wrap=%u,%u lodBias=%g lod=%g,%g",stage,static_cast<unsigned long>(h),static_cast<unsigned long>(removed),desc.Width,desc.Height,desc.MipLevels,static_cast<unsigned long>(device.GetFeatureLevel()),unsigned(source.format),unsigned(source.minFilter),unsigned(source.magFilter),unsigned(source.wrapS),unsigned(source.wrapT),double(source.commandLod?source.commandLodBias:source.lodBias/100.f),double(source.commandLod?source.commandMinLod:source.minLod/8.f),double(source.commandLod?source.commandMaxLod:source.maxLod/8.f));
  if(samplerState&&used>=0&&size_t(used)<sizeof(message))std::snprintf(message+used,sizeof(message)-size_t(used)," samplerFilter=0x%X samplerLodBias=%g samplerLod=%g,%g",unsigned(samplerState->Filter),double(samplerState->MipLODBias),double(samplerState->MinLOD),double(samplerState->MaxLOD));
  throw std::runtime_error(message);
 }
};

