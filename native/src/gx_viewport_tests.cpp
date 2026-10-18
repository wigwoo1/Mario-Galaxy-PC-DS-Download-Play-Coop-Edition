#include <Native/GxViewport.hpp>
#include <Native/D3dModelDraw.hpp>
#include <revolution/gx/GXTransform.h>
#include <revolution/gd/GDBase.h>
#include <cassert>
#include <cstdio>
int main(){
 u8 bytes[29];GDLObj writer;GDInitGDLObj(&writer,bytes,29);GDSetCurrent(&writer);
 GXSetViewport(16,8,32,24,.25f,.75f);Native::GxCommandState state;state.consume(bytes,29);auto v=state.viewport();
 assert((v==std::array<float,6>{16,8,32,24,.25f,.75f}));
 GDInitGDLObj(&writer,bytes,29);GXSetViewportJitter(16,8,32,24,.25f,.75f,0);state.consume(bytes,29);assert(state.viewport()[1]==7.5f);
 GDInitGDLObj(&writer,bytes,29);GXSetViewport(16,8,32,24,.25f,.75f);state.consume(bytes,29);
 GDInitGDLObj(&writer,bytes,28);bool rejected=false;try{GXSetViewport(0,0,64,64,0,1);}catch(const std::exception&){rejected=true;}
 float retained[6];GXGetViewportv(retained);assert(rejected&&GDGetCurrOffset()==0&&retained[0]==16);
 Microsoft::WRL::ComPtr<ID3D11Device> device;Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
 assert(SUCCEEDED(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,nullptr,&context)));
 Native::D3dEfb efb(*context.Get(),64,64);Native::D3dModelDraw draw(*context.Get());draw.setViewport(state);
 float clear[4]={0,0,0,1};efb.clear(clear,1);
 std::vector<Native::ProjectedVertex> triangle={{{-1,-1,-.5f,1},{1,0,0,1}},{{0,1,-.5f,1},{1,0,0,1}},{{1,-1,-.5f,1},{1,0,0,1}}};draw.draw(efb,triangle);
 auto pixels=efb.readColor();assert(pixels[(20*64+32)*4]==255 && pixels[(40*64+32)*4]==0 && pixels[(20*64+8)*4]==0);
 efb.clear(clear,.4f);draw.draw(efb,triangle);pixels=efb.readColor();assert(pixels[(20*64+32)*4]==0);
 for(auto& vertex:triangle)vertex.position[2]=-.9f;draw.draw(efb,triangle);pixels=efb.readColor();assert(pixels[(20*64+32)*4]==255);
 D3D11_VIEWPORT actual{};UINT count=1;context->RSGetViewports(&count,&actual);assert(actual.TopLeftX==16&&actual.TopLeftY==8&&actual.Width==32&&actual.Height==24&&actual.MinDepth==.25f&&actual.MaxDepth==.75f);
 GDSetCurrent(nullptr);std::puts("PASS GX viewport XF decoding, field jitter, atomic rejection and authored WARP viewport rasterization/depth range");
}


