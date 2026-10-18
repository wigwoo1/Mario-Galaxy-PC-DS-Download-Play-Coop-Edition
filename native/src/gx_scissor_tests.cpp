#include <Native/D3dModelDraw.hpp>
#include <revolution/gx/GXTransform.h>
#include <revolution/gd/GDBase.h>
#include <cassert>
#include <cstdio>
int main(){
 u8 bytes[15]{};GDLObj writer;GDInitGDLObj(&writer,bytes,15);GDSetCurrent(&writer);
 GXSetScissor(16,8,24,32);GXSetScissorBoxOffset(0,0);Native::GxCommandState state;state.consume(bytes,15);
 assert((state.scissorRectangle()==std::array<int,4>{16,8,40,40}));
 GDInitGDLObj(&writer,bytes,5);GXSetScissorBoxOffset(3,-3);state.consume(bytes,5);
 assert((state.scissorRectangle()==std::array<int,4>{14,12,38,44}));
 GDInitGDLObj(&writer,bytes,5);GXSetScissorBoxOffset(0,0);state.consume(bytes,5);
 GDInitGDLObj(&writer,bytes,9);bool rejected=false;try{GXSetScissor(0,0,64,64);}catch(const std::exception&){rejected=true;}
 u32 l,t,w,h;GXGetScissor(&l,&t,&w,&h);assert(rejected&&GDGetCurrOffset()==0&&l==16&&w==24);
 Microsoft::WRL::ComPtr<ID3D11Device> device;Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
 assert(SUCCEEDED(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,nullptr,&context)));
 Native::D3dEfb efb(*context.Get(),64,64);Native::D3dModelDraw draw(*context.Get());draw.setScissor(state);
 float clear[4]={0,0,0,1};efb.clear(clear,1);
 std::vector<Native::ProjectedVertex> geometry={{{-1,-1,-.5f,1},{1,0,0,1}},{{-1,1,-.5f,1},{1,0,0,1}},{{1,1,-.5f,1},{1,0,0,1}},{{-1,-1,-.5f,1},{1,0,0,1}},{{1,1,-.5f,1},{1,0,0,1}},{{1,-1,-.5f,1},{1,0,0,1}}};draw.draw(efb,geometry);
 auto pixels=efb.readColor();unsigned red=0;for(unsigned y=0;y<64;++y)for(unsigned x=0;x<64;++x){bool inside=x>=16&&x<40&&y>=8&&y<40;assert((pixels[(y*64+x)*4]==255)==inside);red+=inside;}
 assert(red==768);draw.resetScissor();efb.clear(clear,1);draw.draw(efb,geometry);pixels=efb.readColor();assert(pixels[0]==255);
 GDSetCurrent(nullptr);std::puts("PASS GX scissor bounds, quantized box offsets, atomic rejection and exact 768-pixel WARP clipping");
}
