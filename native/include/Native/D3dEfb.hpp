#pragma once
#include <Native/D3dCompletion.hpp>
#include <vector>
#include <cstring>
#include <cmath>
namespace Native {
class D3dEfb {
public:
 D3dEfb(ID3D11DeviceContext& source,unsigned width,unsigned height):width(width),height(height),context(&source){
  if(!width||!height||width>8192||height>8192)throw std::invalid_argument("Invalid native EFB dimensions");
  context->GetDevice(&device);
  D3D11_TEXTURE2D_DESC desc{};desc.Width=width;desc.Height=height;desc.MipLevels=1;desc.ArraySize=1;desc.SampleDesc.Count=1;desc.Usage=D3D11_USAGE_DEFAULT;
  desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;desc.BindFlags=D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
  check(device->CreateTexture2D(&desc,nullptr,&color));
  check(device->CreateRenderTargetView(color.Get(),nullptr,&colorTarget));
  desc.Format=DXGI_FORMAT_R32_TYPELESS;desc.BindFlags=D3D11_BIND_DEPTH_STENCIL;
  check(device->CreateTexture2D(&desc,nullptr,&depth));
  D3D11_DEPTH_STENCIL_VIEW_DESC view{};view.Format=DXGI_FORMAT_D32_FLOAT;view.ViewDimension=D3D11_DSV_DIMENSION_TEXTURE2D;check(device->CreateDepthStencilView(depth.Get(),&view,&depthTarget));
 }
 void bind(){auto* target=colorTarget.Get();context->OMSetRenderTargets(1,&target,depthTarget.Get());}
 void clear(const float rgba[4],float z){
  if(!rgba||!std::isfinite(z)||z<0||z>1)throw std::invalid_argument("Invalid EFB clear depth");
  for(unsigned i=0;i<4;++i)if(!std::isfinite(rgba[i]))throw std::invalid_argument("Invalid EFB clear color");
  context->ClearRenderTargetView(colorTarget.Get(),rgba);context->ClearDepthStencilView(depthTarget.Get(),D3D11_CLEAR_DEPTH,z,0);
 }
 std::vector<std::uint8_t> readColor(){
  D3D11_TEXTURE2D_DESC desc{};color->GetDesc(&desc);desc.Usage=D3D11_USAGE_STAGING;desc.BindFlags=0;desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> staging;check(device->CreateTexture2D(&desc,nullptr,&staging));
  context->CopyResource(staging.Get(),color.Get());waitForD3dCompletion(*context.Get(),std::chrono::milliseconds(2000));
  std::vector<std::uint8_t> bytes(std::size_t(width)*height*4);
  D3D11_MAPPED_SUBRESOURCE mapped{};check(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped));
  for(unsigned row=0;row<height;++row)std::memcpy(bytes.data()+std::size_t(row)*width*4,static_cast<const std::uint8_t*>(mapped.pData)+std::size_t(row)*mapped.RowPitch,std::size_t(width)*4);
  context->Unmap(staging.Get(),0);return bytes;
 }
 std::uint32_t readArgb(unsigned x,unsigned y){
  if(x>=width||y>=height)throw std::out_of_range("GX EFB pixel coordinate");
  D3D11_TEXTURE2D_DESC desc{};color->GetDesc(&desc);desc.Width=1;desc.Height=1;desc.Usage=D3D11_USAGE_STAGING;desc.BindFlags=0;desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> staging;check(device->CreateTexture2D(&desc,nullptr,&staging));
  D3D11_BOX box{x,y,0,x+1,y+1,1};context->CopySubresourceRegion(staging.Get(),0,0,0,0,color.Get(),0,&box);
  waitForD3dCompletion(*context.Get(),std::chrono::milliseconds(2000));
  D3D11_MAPPED_SUBRESOURCE mapped{};check(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped));
  const auto* rgba=static_cast<const std::uint8_t*>(mapped.pData);
  const std::uint32_t argb=(std::uint32_t(rgba[3])<<24)|(std::uint32_t(rgba[0])<<16)|(std::uint32_t(rgba[1])<<8)|rgba[2];
  context->Unmap(staging.Get(),0);return argb;
 }
 std::vector<float> readDepth(){
  D3D11_TEXTURE2D_DESC desc{};depth->GetDesc(&desc);desc.Usage=D3D11_USAGE_STAGING;desc.BindFlags=0;desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> staging;check(device->CreateTexture2D(&desc,nullptr,&staging));
  context->CopyResource(staging.Get(),depth.Get());waitForD3dCompletion(*context.Get(),std::chrono::milliseconds(2000));
  std::vector<float> values(std::size_t(width)*height);
  D3D11_MAPPED_SUBRESOURCE mapped{};check(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped));
  for(unsigned row=0;row<height;++row)std::memcpy(values.data()+std::size_t(row)*width,static_cast<const std::uint8_t*>(mapped.pData)+std::size_t(row)*mapped.RowPitch,std::size_t(width)*sizeof(float));
  context->Unmap(staging.Get(),0);return values;
 }
 unsigned getWidth()const{return width;}unsigned getHeight()const{return height;}
 ID3D11RenderTargetView* renderTarget()const{return colorTarget.Get();}
 ID3D11DepthStencilView* depthStencil()const{return depthTarget.Get();}
private:
 static void check(HRESULT result){if(FAILED(result))throw std::runtime_error("Native EFB Direct3D operation failed");}
 unsigned width,height;
 Microsoft::WRL::ComPtr<ID3D11Device> device;Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
 Microsoft::WRL::ComPtr<ID3D11Texture2D> color,depth;
 Microsoft::WRL::ComPtr<ID3D11RenderTargetView> colorTarget;
 Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthTarget;
};
}

