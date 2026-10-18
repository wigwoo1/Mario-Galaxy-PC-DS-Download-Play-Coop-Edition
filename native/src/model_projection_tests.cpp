#include "native_model_projection.hpp"
#include <cassert>
#include <cstdio>
int main(){
 Native::GxCommandState rasterState;const std::uint8_t textureOnly[]={0x61,0xc0,0,0,0xff,0x61,0xc1,0,0,0xff};rasterState.consume(textureOnly,sizeof(textureOnly));assert(!gxTevConsumesRasterColor(rasterState));
 const std::uint8_t rasterRgb[]={0x61,0xc0,0,0,0xfa};rasterState.consume(rasterRgb,sizeof(rasterRgb));assert(gxTevConsumesRasterColor(rasterState));
 Native::GxCommandState rasterAlphaState;const std::uint8_t rasterAlpha[]={0x61,0xc1,0,0,0x55};rasterAlphaState.consume(rasterAlpha,sizeof(rasterAlpha));assert(gxTevConsumesRasterColor(rasterAlphaState));
 TProj3f projection;projection.makePerspective(90,1,1,100);
 std::vector<NativeShapeVertex> vertices(3);vertices[0].position={-1,-1,-2};vertices[1].position={0,1,-2};vertices[2].position={1,-1,-2};
 std::vector<std::array<u32,3>> triangles{{0,1,2}};
 auto projected=projectModelTriangles(vertices,triangles,projection,{1,0,0,1});
 assert(projected.size()==3);
 for(unsigned i=0;i<3;++i){assert(std::fabs(projected[i].position[0]-vertices[i].position[0])<1e-5f);assert(projected[i].position[3]==2);assert(std::fabs(projected[i].position[2]+98.f/99)<1e-5f);}
 Microsoft::WRL::ComPtr<ID3D11Device> device;Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
 assert(SUCCEEDED(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,nullptr,&context)));
 Native::D3dEfb efb(*context.Get(),64,64);Native::D3dModelDraw draw(*context.Get());float clear[4]={0,0,0,1};efb.clear(clear,1);draw.draw(efb,projected);
 auto pixels=efb.readColor();assert(pixels[(32*64+32)*4]==255);
 triangles[0][2]=3;bool rejected=false;try{projectModelTriangles(vertices,triangles,projection,{1,0,0,1});}catch(const std::out_of_range&){rejected=true;}assert(rejected);
 std::puts("PASS model projection bridge: original perspective, triangle expansion, actual GPU draw/readback and bounds rejection");
}
