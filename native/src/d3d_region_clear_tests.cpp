#include <Native/D3dEfbRegionClear.hpp>
#include <cassert>
#include <cstdio>
int main(){
 Microsoft::WRL::ComPtr<ID3D11Device> device;Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
 assert(SUCCEEDED(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,nullptr,&context)));
 Native::D3dEfb efb(*context.Get(),17,13);float blue[]={0,0,1,1},red[]={1,0,0,.5f};efb.clear(blue,.25f);
 Native::clearD3dEfbRegion(*context.Get(),efb,3,4,5,6,red,.75f);
 auto color=efb.readColor();auto depth=efb.readDepth();
 for(unsigned y=0;y<13;++y)for(unsigned x=0;x<17;++x){size_t p=y*17+x;bool inside=x>=3&&x<8&&y>=4&&y<10;assert(depth[p]==(inside?.75f:.25f));assert(color[p*4]==(inside?255:0)&&color[p*4+2]==(inside?0:255)&&color[p*4+3]==(inside?128:255));}
 bool rejected=false;try{Native::clearD3dEfbRegion(*context.Get(),efb,16,0,2,1,red,1);}catch(const std::invalid_argument&){rejected=true;}assert(rejected&&efb.readDepth()==depth&&efb.readColor()==color);
 std::puts("PASS GPU region clear color/alpha/depth and exact outside preservation on odd-width surface");
}
