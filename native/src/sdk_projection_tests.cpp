#include <Native/D3dModelDraw.hpp>
#include <revolution/mtx.h>
#include <revolution/gx/GXTransform.h>
#include <revolution/gd/GDBase.h>
#include <cassert>
#include <cstdio>
static float projectZ(const Mtx44 m,float z){return (m[2][2]*z+m[2][3])/(m[3][2]*z+m[3][3]);}
int main(){
 Mtx44 m;C_MTXPerspective(m,90,2,2,10);
 assert(std::fabs(m[0][0]-.5f)<.00001f&&std::fabs(m[1][1]-1)<.00001f);
 assert(projectZ(m,-2)==-1&&projectZ(m,-10)==0);
 C_MTXOrtho(m,0,480,0,640,2,10);
 assert(std::fabs(m[0][0]*640+m[0][3]-1)<.00001f&&m[0][3]==-1);
 assert(std::fabs(m[1][1]*480+m[1][3]+1)<.00001f&&m[1][3]==1);
 assert(projectZ(m,-2)==-1&&projectZ(m,-10)==0);
 u8 bytes[33];GDLObj writer;GDInitGDLObj(&writer,bytes,33);GDSetCurrent(&writer);GXSetProjection(m,GX_ORTHOGRAPHIC);
 Native::GxCommandState state;state.consume(bytes,33);auto decoded=state.projectionMatrix();for(unsigned r=0;r<4;++r)for(unsigned c=0;c<4;++c)assert(decoded[r][c]==m[r][c]);
 Microsoft::WRL::ComPtr<ID3D11Device> device;Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
 assert(SUCCEEDED(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,nullptr,&context)));
 Native::D3dEfb efb(*context.Get(),64,48);Native::D3dModelDraw draw(*context.Get());float clear[4]={0,0,0,1};efb.clear(clear,1);
 const std::array<std::array<float,3>,6> positions={{{160,120,-6},{480,120,-6},{480,360,-6},{160,120,-6},{480,360,-6},{160,360,-6}}};
 std::vector<Native::ProjectedVertex> vertices;
 for(auto pos:positions){Native::ProjectedVertex v{};for(unsigned r=0;r<4;++r)v.position[r]=decoded[r][0]*pos[0]+decoded[r][1]*pos[1]+decoded[r][2]*pos[2]+decoded[r][3];v.color[0]=v.color[3]=1;vertices.push_back(v);}
 draw.draw(efb,vertices);auto pixels=efb.readColor();unsigned drawn=0;for(unsigned y=0;y<48;++y)for(unsigned x=0;x<64;++x){bool inside=x>=16&&x<48&&y>=12&&y<36;assert((pixels[(y*64+x)*4]==255)==inside);drawn+=inside;}assert(drawn==768);
 GDSetCurrent(nullptr);std::puts("PASS original SDK perspective/orthographic math, near/far clip endpoints, GX roundtrip and 768-pixel WARP display rectangle");
}
