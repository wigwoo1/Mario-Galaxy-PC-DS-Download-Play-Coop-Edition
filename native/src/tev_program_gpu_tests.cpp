#include <Native/D3dEfb.hpp>
#include <Native/TevArithmetic.hpp>
#include <Native/TevProgramHlsl.hpp>
#include <Native/TevStage.hpp>
#include <d3dcompiler.h>
#include <iostream>
#include <string>
using Input=Native::TevFragmentInputs;
int main(){try{
 using Microsoft::WRL::ComPtr;
 auto check=[](HRESULT h){if(FAILED(h))throw std::runtime_error("D3D TEV check failed");};
 ComPtr<ID3D11Device> device;ComPtr<ID3D11DeviceContext> context;
 check(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,nullptr,&context));
 Native::GxCommandState state;
 auto bp=[&](unsigned reg,unsigned value){unsigned word=(reg<<24)|(value&0xffffff);unsigned char bytes[]={0x61,(unsigned char)(word>>24),(unsigned char)(word>>16),(unsigned char)(word>>8),(unsigned char)word};state.consume(bytes,5);};
 bp(0,15<<10);
 for(unsigned i=0;i<8;++i)bp(0x28+i,0x40|(0x40<<12));
 for(unsigned i=0;i<8;++i)bp(0xf6+i,(i*37+19)&0xffffff);
 for(unsigned i=0;i<4;++i){bp(0xe0+i*2,((i*143+77)&2047)|(((i*61+29)&2047)<<12));bp(0xe1+i*2,((i*227+31)&2047)|(((i*101+83)&2047)<<12));}
 for(unsigned i=0;i<16;++i){
  unsigned bias=i%4,scale=(i/2)%4,sub=i%2,cl=(i/3)%2;
  bp(0xc0+i*2,((i%16)<<12)|(((i+5)%16)<<8)|(((i+9)%16)<<4)|((i+3)%16)|(bias<<16)|(sub<<18)|(cl<<19)|(scale<<20)|((i%4)<<22));
  bp(0xc1+i*2,(i%4)|(((i+1)%4)<<2)|((i%8)<<13)|(((i+3)%8)<<10)|(((i+5)%8)<<7)|(((i+1)%8)<<4)|(bias<<16)|(sub<<18)|(cl<<19)|(scale<<20)|(((i+1)%4)<<22));
 }
 std::vector<Input> inputs(256);std::vector<int> expected;
 for(unsigned i=0;i<inputs.size();++i){
  for(unsigned j=0;j<16;++j)for(unsigned c=0;c<4;++c){inputs[i].textures[j][c]=(i*73+j*29+c*61)&255;inputs[i].raster[j][c]=(i*151+j*37+c*43)&255;}
  auto result=Native::evaluateTevProgram(state,inputs[i]);expected.insert(expected.end(),result.begin(),result.end());
 }
 for(bool uniform:{false,true}){
 std::string shader=Native::tevProgramHlsl(state,uniform);
 shader+=R"(
 struct Input{int4 textures[16];int4 raster[16];};
 StructuredBuffer<Input> source:register(t0);RWStructuredBuffer<int> output:register(u0);
 [numthreads(1,1,1)]void main(uint3 id:SV_DispatchThreadID){
 Input v=source[id.x];int4 result=evaluateTevProgram(v.textures,v.raster);
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
 ComPtr<ID3D11Buffer> constants;
 if(uniform){auto values=Native::tevProgramConstants(state);D3D11_BUFFER_DESC d{};d.ByteWidth=sizeof(values);d.Usage=D3D11_USAGE_IMMUTABLE;d.BindFlags=D3D11_BIND_CONSTANT_BUFFER;D3D11_SUBRESOURCE_DATA initial{&values,0,0};check(device->CreateBuffer(&d,&initial,&constants));}
 auto* constant=constants.Get();context->CSSetConstantBuffers(0,1,&constant);
 context->CSSetShader(cs.Get(),nullptr,0);auto* s=srv.Get();auto* u=uav.Get();context->CSSetShaderResources(0,1,&s);context->CSSetUnorderedAccessViews(0,1,&u,nullptr);context->Dispatch(UINT(inputs.size()),1,1);
 u=nullptr;context->CSSetUnorderedAccessViews(0,1,&u,nullptr);
 desc.Usage=D3D11_USAGE_STAGING;desc.BindFlags=0;desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;desc.MiscFlags=0;desc.StructureByteStride=0;
 ComPtr<ID3D11Buffer> staging;check(device->CreateBuffer(&desc,nullptr,&staging));context->CopyResource(staging.Get(),output.Get());
 Native::waitForD3dCompletion(*context.Get(),std::chrono::milliseconds(5000));
 D3D11_MAPPED_SUBRESOURCE mapped{};check(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&mapped));
 auto* actual=(const int*)mapped.pData;bool equal=true;for(size_t i=0;i<expected.size();++i)if(actual[i]!=expected[i]){std::cerr<<"Mismatch "<<i<<" expected "<<expected[i]<<" actual "<<actual[i]<<"\n";equal=false;break;}
 context->Unmap(staging.Get(),0);if(!equal)return 1;
 std::cout<<"PASS GPU 16-stage TEV program "<<inputs.size()<<" fragments ("<<(uniform?"constant buffer":"literal reference")<<")\n";
 }return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}}


