#include "gx_command_texture.hpp"
#include "native_model_texture.hpp"
#include <cassert>
#include <cstdio>
int main(){
 Native::GxCommandState state;Native::GpuResources resources;
 std::vector<u8> bytes(96);for(unsigned i=0;i<96;++i)bytes[i]=i<32?13:i<64?71:149;
 auto address=resources.add(bytes.data(),bytes.size());
 auto bp=[&](unsigned reg,unsigned v){std::vector<u8> b{0x61,u8(reg),u8(v>>16),u8(v>>8),u8(v)};state.consume(b.data(),b.size());};
 bp(0x88,3|(3<<10)|(1<<20));bp(0x94,address>>5);bp(0x80,(6<<5)|(1<<4)|(unsigned(216)<<9));bp(0x84,9|(32<<8));
 auto texture=decodeCommandTexture(state,0,resources);assert(texture.mipmaps.size()==2&&texture.commandMinLod==.5625f&&texture.commandMaxLod==2&&texture.commandLodBias==-1.25f);
 assert(texture.pixels[0].r==13&&texture.mipmaps[0].pixels[0].r==71&&texture.mipmaps[1].pixels[0].r==149);
 Microsoft::WRL::ComPtr<ID3D11Device> device;assert(SUCCEEDED(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,nullptr,nullptr)));
 NativeModelTexture uploaded(*device.Get(),texture);D3D11_SAMPLER_DESC desc{};uploaded.sampler->GetDesc(&desc);assert(desc.MinLOD==.5625f&&desc.MaxLOD==2&&desc.MipLODBias==-1.25f);
 bp(0x84,9|(17<<8));texture=decodeCommandTexture(state,0,resources);assert(texture.mipmaps.size()==2&&texture.commandMaxLod==1.0625f);
 resources.remove(bytes.data());bytes.resize(32);auto shortAddress=resources.add(bytes.data(),bytes.size());bp(0x94,shortAddress>>5);bool rejected=false;try{decodeCommandTexture(state,0,resources);}catch(const std::exception&){rejected=true;}assert(rejected);
 std::puts("PASS command mip decoding: tiled levels, fractional LOD/bias, GPU sampler and truncated-chain rejection");
}

