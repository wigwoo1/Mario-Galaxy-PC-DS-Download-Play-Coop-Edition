#include <Native/D3dEfb.hpp>
#include <Native/TevArithmetic.hpp>
#include <Native/TevArithmeticHlsl.hpp>
#include <d3dcompiler.h>
#include <iostream>
#include <string>
struct Input {int a,b,c,d;unsigned bias,sub,scale,clampResult;int ca[3],cb[3],cc[3],cd[3];unsigned mode,operation;};
int main(){try{
 using Microsoft::WRL::ComPtr;
 auto check=[](HRESULT h){if(FAILED(h))throw std::runtime_error("D3D TEV check failed");};
 ComPtr<ID3D11Device> device;ComPtr<ID3D11DeviceContext> context;
 check(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,nullptr,&context));
 std::vector<Input> inputs;std::vector<int> expected;
 for(unsigned bias=0;bias<3;++bias)for(unsigned sub=0;sub<2;++sub)for(unsigned scale=0;scale<4;++scale)for(unsigned cl=0;cl<2;++cl)
 for(int c:{0,1,127,128,254,255})for(int d:{-1024,-129,-1,0,127,1023}){
  int a=(inputs.size()*73)&255,b=(inputs.size()*151+255)&255;
  inputs.push_back({a,b,c,d,bias,sub,scale,cl});expected.push_back(Native::tevArithmetic(a,b,c,d,bias,sub,scale,cl));
 }
  const size_t normalCount=inputs.size();
 // Packed compare channel significance, equality, per-channel and saturation.
 for(unsigned op=0;op<8;++op)for(unsigned cl=0;cl<2;++cl)
 for(unsigned pattern=0;pattern<12;++pattern)for(int d:{-1024,-1,0,255,1023}){
  Native::TevRgb a{13,71,149},b=a,c{1,128,255},dest{d,d,d};
  if(pattern<6)b[pattern/2]+=pattern%2?1:-1;
  else if(pattern<9){b[pattern-6]+=1;b[(pattern-6+1)%3]-=1;}
  else if(pattern==10)b={0,0,0};
  else if(pattern==11)b={255,255,255};
  Input v{};v.a=pattern&1?255:0;v.b=pattern&2?255:0;v.c=128;v.d=d;v.clampResult=cl;v.mode=1;v.operation=op;
  for(unsigned j=0;j<3;++j){v.ca[j]=a[j];v.cb[j]=b[j];v.cc[j]=c[j];v.cd[j]=dest[j];}
  // Each input writes four scalar slots, keeping the storage ABI explicit.
  inputs.push_back(v);
 }
 std::vector<int> expanded;expanded.reserve(inputs.size()*4);
 for(size_t i=0;i<inputs.size();++i){
  auto& v=inputs[i];
  if(!v.mode){for(unsigned j=0;j<4;++j)expanded.push_back(expected[i]);}
  else{
   Native::TevRgb a{v.ca[0],v.ca[1],v.ca[2]},b{v.cb[0],v.cb[1],v.cb[2]},c{v.cc[0],v.cc[1],v.cc[2]},d{v.cd[0],v.cd[1],v.cd[2]};
   auto result=Native::tevColorCompare(a,b,c,d,v.operation,v.clampResult);
   expanded.insert(expanded.end(),result.begin(),result.end());
   expanded.push_back(Native::tevAlphaCompare(a,b,v.a,v.b,v.c,v.d,v.operation,v.clampResult));
  }
 }
 expected=std::move(expanded);
 std::string shader=Native::tevArithmeticHlsl;
 shader+=R"(
 struct Input{int a,b,c,d;uint bias,sub,scale,clampResult;int3 ca,cb,cc,cd;uint mode,operation;};
 StructuredBuffer<Input> source:register(t0);RWStructuredBuffer<int> output:register(u0);
 [numthreads(1,1,1)]void main(uint3 id:SV_DispatchThreadID){
  Input v=source[id.x];int4 result;
 if(v.mode==0){int value=tevArithmetic(v.a,v.b,v.c,v.d,v.bias,v.sub!=0,v.scale,v.clampResult!=0);result=int4(value,value,value,value);}
 else result=int4(tevColorCompare(v.ca,v.cb,v.cc,v.cd,v.operation,v.clampResult!=0),tevAlphaCompare(v.ca,v.cb,v.a,v.b,v.c,v.d,v.operation,v.clampResult!=0));
 for(uint j=0;j<4;++j)output[id.x*4+j]=result[j];
 })";
 ComPtr<ID3DBlob> code,error;
 HRESULT compiled=D3DCompile(shader.data(),shader.size(),nullptr,nullptr,nullptr,"main","cs_5_0",D3DCOMPILE_ENABLE_STRICTNESS,0,&code,&error);
 if(FAILED(compiled)){if(error)std::cerr<<(const char*)error->GetBufferPointer();check(compiled);}
 ComPtr<ID3D11ComputeShader> cs;check(device->CreateComputeShader(code->GetBufferPointer(),code->GetBufferSize(),nullptr,&cs));
 D3D11_BUFFER_DESC desc{};desc.ByteWidth=UINT(inputs.size()*sizeof(Input));desc.Usage=D3D11_USAGE_IMMUTABLE;desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;desc.MiscFlags=D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;desc.StructureByteStride=sizeof(Input);
 D3D11_SUBRESOURCE_DATA data{inputs.data(),0,0};ComPtr<ID3D11Buffer> source;check(device->CreateBuffer(&desc,&data,&source));
 ComPtr<ID3D11ShaderResourceView> srv;check(device->CreateShaderResourceView(source.Get(),nullptr,&srv));
 desc.ByteWidth=UINT(expected.size()*sizeof(int));desc.Usage=D3D11_USAGE_DEFAULT;desc.BindFlags=D3D11_BIND_UNORDERED_ACCESS;desc.StructureByteStride=sizeof(int);
 ComPtr<ID3D11Buffer> output;check(device->CreateBuffer(&desc,nullptr,&output));ComPtr<ID3D11UnorderedAccessView> uav;check(device->CreateUnorderedAccessView(output.Get(),nullptr,&uav));
 context->CSSetShader(cs.Get(),nullptr,0);auto* s=srv.Get();auto* u=uav.Get();context->CSSetShaderResources(0,1,&s);context->CSSetUnorderedAccessViews(0,1,&u,nullptr);context->Dispatch(UINT(inputs.size()),1,1);
 u=nullptr;context->CSSetUnorderedAccessViews(0,1,&u,nullptr);
 desc.Usage=D3D11_USAGE_STAGING;desc.BindFlags=0;desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;desc.MiscFlags=0;desc.StructureByteStride=0;
 ComPtr<ID3D11Buffer> staging;check(device->CreateBuffer(&desc,nullptr,&staging));context->CopyResource(staging.Get(),output.Get());
 Native::waitForD3dCompletion(*context.Get(),std::chrono::milliseconds(5000));
 D3D11_MAPPED_SUBRESOURCE mapped{};check(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped));
 auto* actual=(const int*)mapped.pData;bool equal=true;for(size_t i=0;i<expected.size();++i)if(actual[i]!=expected[i]){std::cerr<<"Mismatch "<<i<<" expected "<<expected[i]<<" actual "<<actual[i]<<"\n";equal=false;break;}
 context->Unmap(staging.Get(),0);if(!equal)return 1;
 std::cout<<"PASS GPU TEV arithmetic "<<normalCount<<" normal and "<<inputs.size()-normalCount<<" comparison cases\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}}

