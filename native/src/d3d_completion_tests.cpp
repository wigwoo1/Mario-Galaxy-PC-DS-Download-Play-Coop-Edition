#include <Native/D3dCompletion.hpp>
#include <cassert>
#include <cstdio>
int main(){
 Microsoft::WRL::ComPtr<ID3D11Device> device;Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
 assert(SUCCEEDED(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,nullptr,&context)));
 D3D11_BUFFER_DESC desc{};desc.ByteWidth=16;desc.Usage=D3D11_USAGE_DEFAULT;
 unsigned values[4]={1,2,3,4};D3D11_SUBRESOURCE_DATA initial{values,0,0};Microsoft::WRL::ComPtr<ID3D11Buffer> source,readback;
 assert(SUCCEEDED(device->CreateBuffer(&desc,&initial,&source)));desc.Usage=D3D11_USAGE_STAGING;desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
 assert(SUCCEEDED(device->CreateBuffer(&desc,nullptr,&readback)));context->CopyResource(readback.Get(),source.Get());
 Native::waitForD3dCompletion(*context.Get(),std::chrono::milliseconds(2000));
 D3D11_MAPPED_SUBRESOURCE mapped{};assert(SUCCEEDED(context->Map(readback.Get(),0,D3D11_MAP_READ,0,&mapped)));for(int i=0;i<4;++i)assert(static_cast<unsigned*>(mapped.pData)[i]==values[i]);context->Unmap(readback.Get(),0);
 std::puts("PASS actual D3D GPU completion: WARP copy fence and readback contents");
}
