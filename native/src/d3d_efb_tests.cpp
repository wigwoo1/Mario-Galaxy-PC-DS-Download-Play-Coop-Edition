#include <Native/D3dDrawSyncCompletion.hpp>
#include <revolution/gx/GXManage.h>
#include <Native/D3dDepthPeek.hpp>
#include <revolution/gx/GXCpu2Efb.h>
#include <Native/D3dEfb.hpp>
#include <Native/EfbReadback.hpp>
#include <cassert>
#include <cstdio>
static unsigned callbackCount;
static void depthCallback(u16 token){assert(token==0xabcd);u32 depth=0;GXPeekZ(16,12,&depth);assert(depth==0x600000);++callbackCount;}
static std::uint32_t readColorPixel(void* target,std::uint16_t x,std::uint16_t y){return static_cast<Native::D3dEfb*>(target)->readArgb(x,y);}
int main(){
 Microsoft::WRL::ComPtr<ID3D11Device> device;Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
 assert(SUCCEEDED(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,nullptr,&context)));
 Native::D3dEfb efb(*context.Get(),17,13);efb.bind();float clear[4]={.25f,.5f,.75f,1};efb.clear(clear,1);
 auto depths=efb.readDepth();assert(depths.size()==17*13);for(float value:depths)assert(value==1);
 auto bytes=efb.readColor();assert(bytes.size()==17*13*4);
 for(unsigned i=0;i<bytes.size();i+=4)assert(bytes[i]==64&&bytes[i+1]==128&&bytes[i+2]==191&&bytes[i+3]==255);
 {Native::EfbReadbackBinding binding(&efb,readColorPixel);float sample[4]={.25f,.5f,.75f,.25f};efb.clear(sample,1);u32 argb=0;
  GXPokeAlphaRead(GX_READ_NONE);GXPeekARGB(16,12,&argb);assert(argb==0x404080bfu);
  GXPokeAlphaRead(GX_READ_FF);GXPeekARGB(16,12,&argb);assert(argb==0xff4080bfu);
  GXPokeAlphaRead(GX_READ_00);GXPeekARGB(16,12,&argb);assert(argb==0x004080bfu);
  argb=0xdeadbeef;bool bad=false;try{GXPeekARGB(17,0,&argb);}catch(const std::out_of_range&){bad=true;}assert(bad&&argb==0xdeadbeefu);
 }
 u32 noRead=0;bool unboundEfb=false;try{GXPeekARGB(0,0,&noRead);}catch(const std::logic_error&){unboundEfb=true;}assert(unboundEfb);
 float second[4]={0,0,0,0};efb.clear(second,0);auto replaced=efb.readColor();for(auto value:replaced)assert(value==0);
 depths=efb.readDepth();for(float value:depths)assert(value==0);
 efb.clear(second,.375f);depths=efb.readDepth();for(float value:depths)assert(value==.375f);
  Native::D3dDepthPeek source(efb);u32 value=0;
 {Native::DepthPeekBinding binding(source);GXPeekZ(16,12,&value);assert(value==0x600000);
  assert(GXSetDrawSyncCallback(depthCallback)==nullptr);Native::completeD3dDrawSync(*context.Get(),0xabcd,std::chrono::milliseconds(2000));assert(callbackCount==1);assert(GXSetDrawSyncCallback(nullptr)==depthCallback);
  value=123;bool bad=false;try{GXPeekZ(17,0,&value);}catch(const std::out_of_range&){bad=true;}assert(bad&&value==123);
  efb.clear(second,1);GXPeekZ(0,0,&value);assert(value==0xffffff);
  efb.clear(second,0);GXPeekZ(0,0,&value);assert(value==0);
 }
 bool unbound=false;try{GXPeekZ(0,0,&value);}catch(const std::logic_error&){unbound=true;}assert(unbound);
 bool rejected=false;try{efb.clear(second,2);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
 rejected=false;try{Native::D3dEfb invalid(*context.Get(),0,13);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
 std::puts("PASS persistent native EFB: color/depth allocation, GX ARGB readback/alpha modes, odd-width row readback and input rejection");
}




