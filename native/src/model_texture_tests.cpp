#include "native_model_texture.hpp"
#include <cassert>
#include <cstdio>
int main(){
 Microsoft::WRL::ComPtr<ID3D11Device> device;Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
 assert(SUCCEEDED(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,nullptr,&context)));
 TextureImage image{4,2,6,std::vector<Pixel>(8,{13,71,149,255})};image.mipmaps={{2,1,{{29,83,227,128},{61,101,173,127}}},{1,1,{{255,0,128,1}}}};
 image.minFilter=5;image.wrapS=2;image.wrapT=0;image.minLod=4;image.maxLod=16;image.lodBias=-125;
 NativeModelTexture uploaded(*device.Get(),image);
 D3D11_SAMPLER_DESC sampler{};uploaded.sampler->GetDesc(&sampler);
 assert(sampler.Filter==D3D11_FILTER_MIN_MAG_MIP_LINEAR&&sampler.AddressU==D3D11_TEXTURE_ADDRESS_MIRROR&&sampler.AddressV==D3D11_TEXTURE_ADDRESS_CLAMP);
 assert(sampler.MinLOD==.5f&&sampler.MaxLOD==2&&sampler.MipLODBias==-1.25f);
 D3D11_TEXTURE2D_DESC desc{};uploaded.texture->GetDesc(&desc);assert(desc.MipLevels==3);
 desc.Usage=D3D11_USAGE_STAGING;desc.BindFlags=0;desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
 Microsoft::WRL::ComPtr<ID3D11Texture2D> staging;assert(SUCCEEDED(device->CreateTexture2D(&desc,nullptr,&staging)));
 context->CopyResource(staging.Get(),uploaded.texture.Get());Native::waitForD3dCompletion(*context.Get(),std::chrono::milliseconds(5000));
 for(unsigned level=0;level<3;++level){D3D11_MAPPED_SUBRESOURCE mapped{};assert(SUCCEEDED(context->Map(staging.Get(),level,D3D11_MAP_READ,0,&mapped)));auto& pixels=level?image.mipmaps[level-1].pixels:image.pixels;unsigned w=level?image.mipmaps[level-1].width:image.width,h=level?image.mipmaps[level-1].height:image.height;for(unsigned y=0;y<h;++y)assert(std::memcmp((const char*)mapped.pData+y*mapped.RowPitch,pixels.data()+y*w,w*4)==0);context->Unmap(staging.Get(),level);}
 image.mipmaps[0].width=3;bool rejected=false;try{NativeModelTexture invalid(*device.Get(),image);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
 // Exercise an actual failing D3D call without exhausting memory: the
 // requested FL9_1 device rejects a texture wider than its API limit.
 Microsoft::WRL::ComPtr<ID3D11Device> limited;
 const D3D_FEATURE_LEVEL requested=D3D_FEATURE_LEVEL_9_1;D3D_FEATURE_LEVEL obtained{};
 assert(SUCCEEDED(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,&requested,1,D3D11_SDK_VERSION,&limited,&obtained,nullptr)));
 assert(obtained==requested);
 const unsigned width=D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION+1;
 TextureImage oversized{width,1,6,std::vector<Pixel>(width,{1,2,3,4})};
 bool detailed=false;
 try{NativeModelTexture invalid(*limited.Get(),oversized);}catch(const std::runtime_error& error){
  const std::string message=error.what();
  detailed=message.find("stage=CreateSamplerState")!=std::string::npos&&message.find("HRESULT=0x80070057")!=std::string::npos&&message.find("deviceReason=0x00000000")!=std::string::npos&&message.find("size=2049x1 levels=1 featureLevel=0x9100 gxFormat=6")!=std::string::npos&&message.find("samplerFilter=0x14 samplerLodBias=0 samplerLod=0,0")!=std::string::npos;
  std::fprintf(stderr,"Observed expected real D3D rejection: %s\n",error.what());
 }
 if(!detailed)std::fprintf(stderr,"Expected detailed failure not observed on FL9_1 texture width=%u\n",width);
 assert(detailed);
 std::puts("PASS actual failed GPU upload reports stage, HRESULT, device reason and texture metadata");
 std::puts("PASS model texture upload: all mip bytes, sampler wrap/filter/LOD and invalid chain rejection");
}
