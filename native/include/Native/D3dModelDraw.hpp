#pragma once
#include <Native/D3dEfb.hpp>
#include <d3dcompiler.h>
#include <array>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <Native/TevProgramHlsl.hpp>
#include <Native/AlphaTestHlsl.hpp>
#include <Native/IndirectTextureHlsl.hpp>
#include <Native/DepthTextureHlsl.hpp>
namespace Native {
struct ProjectedVertex {float position[4];float color[4];float uv[3]={0,0,1};float extraUv[7][3]={{0,0,1},{0,0,1},{0,0,1},{0,0,1},{0,0,1},{0,0,1},{0,0,1}};};
namespace detail {
struct D3dModelShaderCache {
 Microsoft::WRL::ComPtr<ID3D11Device> device;
 Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex;
 Microsoft::WRL::ComPtr<ID3D11PixelShader> defaultPixel;
 Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
 std::unordered_map<std::string,Microsoft::WRL::ComPtr<ID3D11PixelShader>> pixelShaders;
};
inline std::shared_ptr<D3dModelShaderCache> d3dModelShaderCache(ID3D11Device* device){
 static std::mutex mutex;
 static std::unordered_map<ID3D11Device*,std::shared_ptr<D3dModelShaderCache>> caches;
 std::lock_guard<std::mutex> lock(mutex);
 auto found=caches.find(device);if(found!=caches.end())return found->second;
 auto cache=std::make_shared<D3dModelShaderCache>();cache->device=device;
 const char* shader="struct V{float4 p:POSITION;float4 c:COLOR;float3 uv[8]:TEXCOORD;};struct O{float4 p:SV_Position;float4 c:COLOR;float3 uv[8]:TEXCOORD;};O vs(V v){O o;o.p=v.p;o.p.z+=o.p.w;o.c=v.c;for(uint i=0;i<8;++i)o.uv[i]=v.uv[i];return o;}float4 ps(O o):SV_Target{return o.c;}";
 auto compile=[&](const char* entry,const char* profile){Microsoft::WRL::ComPtr<ID3DBlob> code,error;HRESULT hr=D3DCompile(shader,std::strlen(shader),"native-model-base",nullptr,nullptr,entry,profile,D3DCOMPILE_ENABLE_STRICTNESS,0,&code,&error);if(FAILED(hr))throw std::runtime_error(error?std::string(static_cast<const char*>(error->GetBufferPointer()),error->GetBufferSize()):"Native model base shader compile failed");return code;};
 auto vertex=compile("vs","vs_4_0"),pixel=compile("ps","ps_4_0");
 HRESULT hr=device->CreateVertexShader(vertex->GetBufferPointer(),vertex->GetBufferSize(),nullptr,&cache->vertex);if(FAILED(hr))throw std::runtime_error("Native model vertex shader creation failed");
 hr=device->CreatePixelShader(pixel->GetBufferPointer(),pixel->GetBufferSize(),nullptr,&cache->defaultPixel);if(FAILED(hr))throw std::runtime_error("Native model pixel shader creation failed");
 D3D11_INPUT_ELEMENT_DESC elements[10]={{"POSITION",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,16,D3D11_INPUT_PER_VERTEX_DATA,0}};
 for(unsigned i=0;i<8;++i)elements[2+i]={"TEXCOORD",i,DXGI_FORMAT_R32G32B32_FLOAT,0,32+i*12,D3D11_INPUT_PER_VERTEX_DATA,0};
 hr=device->CreateInputLayout(elements,10,vertex->GetBufferPointer(),vertex->GetBufferSize(),&cache->inputLayout);if(FAILED(hr))throw std::runtime_error("Native model input layout creation failed");
 caches.emplace(device,cache);return cache;
}
inline Microsoft::WRL::ComPtr<ID3D11PixelShader> d3dModelPixelShader(ID3D11Device* device,const std::string& source){
 auto cache=d3dModelShaderCache(device);
 static std::mutex mutex;std::lock_guard<std::mutex> lock(mutex);
 auto found=cache->pixelShaders.find(source);if(found!=cache->pixelShaders.end())return found->second;
 Microsoft::WRL::ComPtr<ID3DBlob> code,error;HRESULT hr=D3DCompile(source.data(),source.size(),"native-model-tev",nullptr,nullptr,"ps","ps_4_0",D3DCOMPILE_ENABLE_STRICTNESS,0,&code,&error);
 if(FAILED(hr))throw std::runtime_error(error?std::string(static_cast<const char*>(error->GetBufferPointer()),error->GetBufferSize()):"TEV pixel shader compilation failed");
 Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel;hr=device->CreatePixelShader(code->GetBufferPointer(),code->GetBufferSize(),nullptr,&pixel);if(FAILED(hr))throw std::runtime_error("TEV pixel shader creation failed");
 if(cache->pixelShaders.size()<512)cache->pixelShaders.emplace(source,pixel);
 return pixel;
}
} // namespace detail
class D3dModelDraw {
public:
 explicit D3dModelDraw(ID3D11DeviceContext& source):context(&source){
  context->GetDevice(&device);
  auto shaders=detail::d3dModelShaderCache(device.Get());vs=shaders->vertex;ps=shaders->defaultPixel;layout=shaders->inputLayout;
  D3D11_RASTERIZER_DESC raster{};raster.FillMode=D3D11_FILL_SOLID;raster.CullMode=D3D11_CULL_NONE;raster.DepthClipEnable=TRUE;raster.ScissorEnable=TRUE;check(device->CreateRasterizerState(&raster,&rasterizer));
  D3D11_DEPTH_STENCIL_DESC depth{};depth.DepthEnable=TRUE;depth.DepthWriteMask=D3D11_DEPTH_WRITE_MASK_ALL;depth.DepthFunc=D3D11_COMPARISON_LESS_EQUAL;check(device->CreateDepthStencilState(&depth,&depthState));
 }
 // Untextured material path. Vertex color must already contain channel-0 lighting.
 // Texture-enabled stages and unsupported raster inputs cannot silently fall back.
 void setUntexturedMaterial(const GxCommandState& state){setMaterial(state,{},{});}
 // Caller supplies generated projective UV channels and prelit raster channel 0.
 void setMaterial(const GxCommandState& state,
                  const std::array<ID3D11ShaderResourceView*,8>& views,
                  const std::array<ID3D11SamplerState*,8>& samplers){
  auto usedTextures=materialTextureSlots(state);
  for(unsigned slot=0;slot<8;++slot)if(usedTextures[slot]&&(!views[slot]||!samplers[slot]))throw std::invalid_argument("Missing model texture/sampler in slot "+std::to_string(slot));
  for(unsigned i=0;i<state.tevStageCount();++i){
   auto order=state.tevOrder(i);
    auto indirect=state.tevIndirect(i);
    if(indirect.alpha)throw std::invalid_argument("Material requires indirect bump alpha at TEV stage "+std::to_string(i));
    if(indirect.matrix&&(!(indirect.matrix&3)||(indirect.matrix>>2)==3))throw std::invalid_argument("Invalid GX indirect matrix selector");
   if(order.channel!=0&&order.channel!=7)throw std::invalid_argument("Material requires another raster channel");
  }
  std::string shader="struct O{float4 p:SV_Position;float4 c:COLOR;float3 uv[8]:TEXCOORD;};";
  for(unsigned slot=0;slot<8;++slot)shader+="Texture2D image"+std::to_string(slot)+":register(t"+std::to_string(slot)+");SamplerState sampling"+std::to_string(slot)+":register(s"+std::to_string(slot)+");";
  shader+=tevProgramHlsl(state,true);
  auto destinationAlpha=state.destinationAlpha();
  const bool textureDepth=writesTextureDepth(state),structuredOutput=destinationAlpha.enabled||textureDepth;
  if(structuredOutput){
   shader+="struct PixelResult{float4 primary:SV_Target0;";
   if(destinationAlpha.enabled)shader+="float4 secondary:SV_Target1;";
   if(textureDepth)shader+="float depth:SV_Depth;";
   shader+="};PixelResult ps(O o){";
  }else shader+="float4 ps(O o):SV_Target{";
  shader+=modelTextureSamplesHlsl(state);
  shader+="int4 rasters[16];int4 color=int4(round(saturate(o.c)*255));";
  for(unsigned i=0;i<16;++i){
   shader+="rasters["+std::to_string(i)+"]=";
   shader+=i<state.tevStageCount()&&state.tevOrder(i).channel==0?"color;":"int4(0,0,0,0);";
  }
  shader+="int4 result=evaluateTevProgram(textures,rasters);"+alphaTestHlsl(state);
  if(structuredOutput){
   shader+="PixelResult output;output.primary=float4(result)/255.0;";
   if(destinationAlpha.enabled)shader+="output.secondary=output.primary;output.primary.a="+std::to_string(destinationAlpha.alpha)+".0/255.0;";
   if(textureDepth)shader+=textureDepthHlsl(state);
   shader+="return output;}";
  }
  else shader+="return float4(result)/255.0;}";
  auto replacement=detail::d3dModelPixelShader(device.Get(),shader);
  auto depthReplacement=createDepthMode(state);
  auto rasterReplacement=createCullMode(state);auto blendReplacement=createBlendMode(state);
  const auto constants=tevProgramConstants(state);
  D3D11_BUFFER_DESC constantDescription{};constantDescription.ByteWidth=sizeof(constants);constantDescription.Usage=D3D11_USAGE_IMMUTABLE;constantDescription.BindFlags=D3D11_BIND_CONSTANT_BUFFER;
  D3D11_SUBRESOURCE_DATA constantData{&constants,0,0};Microsoft::WRL::ComPtr<ID3D11Buffer> constantReplacement;
  check(device->CreateBuffer(&constantDescription,&constantData,&constantReplacement));
  ps=std::move(replacement);blendState=std::move(blendReplacement);depthState=std::move(depthReplacement);rasterizer=std::move(rasterReplacement);cullAll=((state.bp(0)>>14)&3)==3;
  tevConstants=std::move(constantReplacement);
  shaderDestinationAlphaEnabled=destinationAlpha.enabled;shaderDestinationAlpha=destinationAlpha.alpha;
  for(unsigned i=0;i<8;++i){materialViews[i]=views[i];materialSamplers[i]=samplers[i];}
 }
 // GX ZMODE: compare enable bit0, function bits1..3, update enable bit4.
 void setDepthMode(const GxCommandState& state){
  auto replacement=createDepthMode(state);depthState=std::move(replacement);
 }
 void setCullMode(const GxCommandState& state){
  auto replacement=createCullMode(state);rasterizer=std::move(replacement);cullAll=((state.bp(0)>>14)&3)==3;
 }
 void setBlendMode(const GxCommandState& state){
  auto alpha=state.destinationAlpha();
  if(alpha.enabled!=shaderDestinationAlphaEnabled||(alpha.enabled&&alpha.alpha!=shaderDestinationAlpha))throw std::invalid_argument("Destination alpha change requires material rebind");
  auto replacement=createBlendMode(state);blendState=std::move(replacement);
 }
 void setViewport(const GxCommandState& state){
  auto v=state.viewport();
  for(float f:v)if(!std::isfinite(f))throw std::invalid_argument("Nonfinite native viewport");
  if(v[2]<=0||v[3]<=0||v[0]<-32768||v[1]<-32768||v[0]+v[2]>32767||v[1]+v[3]>32767||v[4]<0||v[5]>1||v[4]>v[5])throw std::invalid_argument("GX viewport outside native supported range");
  authoredViewport={v[0],v[1],v[2],v[3],v[4],v[5]};hasViewport=true;
 }
 void setScissor(const GxCommandState& state){auto r=state.scissorRectangle();authoredScissor={r[0],r[1],std::max(r[0],r[2]),std::max(r[1],r[3])};hasScissor=true;}
 void setNativeScissor(unsigned left,unsigned top,unsigned right,unsigned bottom){
  if(left>right||top>bottom||right>unsigned(LONG_MAX)||bottom>unsigned(LONG_MAX))throw std::invalid_argument("Invalid native scissor");
  authoredScissor={LONG(left),LONG(top),LONG(right),LONG(bottom)};hasScissor=true;
 }
 void resetScissor(){hasScissor=false;}
 void resetViewport(){hasViewport=false;}
 void draw(D3dEfb& efb,const std::vector<ProjectedVertex>& vertices){
  if(vertices.empty()||cullAll)return;
  if(vertices.size()%3||vertices.size()>UINT32_MAX/sizeof(ProjectedVertex))throw std::invalid_argument("Invalid model triangle vertices");
  for(const auto& vertex:vertices)for(float value:vertex.position)if(!std::isfinite(value))throw std::invalid_argument("Nonfinite model position");
  D3D11_BUFFER_DESC description{};description.ByteWidth=UINT(vertices.size()*sizeof(ProjectedVertex));description.Usage=D3D11_USAGE_IMMUTABLE;description.BindFlags=D3D11_BIND_VERTEX_BUFFER;
  D3D11_SUBRESOURCE_DATA data{vertices.data(),0,0};Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;check(device->CreateBuffer(&description,&data,&buffer));
  efb.bind();D3D11_VIEWPORT viewport{0,0,float(efb.getWidth()),float(efb.getHeight()),0,1};if(hasViewport)viewport=authoredViewport;context->RSSetViewports(1,&viewport);context->RSSetState(rasterizer.Get());D3D11_RECT clip{0,0,LONG(efb.getWidth()),LONG(efb.getHeight())};if(hasScissor)clip=authoredScissor;context->RSSetScissorRects(1,&clip);
  context->OMSetDepthStencilState(depthState.Get(),0);context->OMSetBlendState(blendState.Get(),nullptr,0xffffffff);
  context->IASetInputLayout(layout.Get());context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);UINT stride=sizeof(ProjectedVertex),offset=0;auto* input=buffer.Get();context->IASetVertexBuffers(0,1,&input,&stride,&offset);
  context->VSSetShader(vs.Get(),nullptr,0);context->PSSetShader(ps.Get(),nullptr,0);ID3D11ShaderResourceView* views[8];ID3D11SamplerState* samplers[8];
  auto* constants=tevConstants.Get();context->PSSetConstantBuffers(0,1,&constants);
  for(unsigned i=0;i<8;++i){views[i]=materialViews[i].Get();samplers[i]=materialSamplers[i].Get();}
  context->PSSetShaderResources(0,8,views);context->PSSetSamplers(0,8,samplers);
  context->Draw(UINT(vertices.size()),0);
 }
private:
 bool cullAll=false;
 bool shaderDestinationAlphaEnabled=false;unsigned shaderDestinationAlpha=0;
 Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;
 Microsoft::WRL::ComPtr<ID3D11BlendState> createBlendMode(const GxCommandState& state){
  unsigned mode=state.bp(0x41);
  if(mode&2)throw std::invalid_argument("GX logic blend requires separate implementation");
  D3D11_BLEND_DESC desc{};auto& target=desc.RenderTarget[0];
  target.RenderTargetWriteMask=((mode&8)?7:0)|((mode&16)?8:0);
  target.BlendEnable=(mode&1)!=0;
  static const D3D11_BLEND source[]={D3D11_BLEND_ZERO,D3D11_BLEND_ONE,D3D11_BLEND_DEST_COLOR,D3D11_BLEND_INV_DEST_COLOR,D3D11_BLEND_SRC_ALPHA,D3D11_BLEND_INV_SRC_ALPHA,D3D11_BLEND_DEST_ALPHA,D3D11_BLEND_INV_DEST_ALPHA};
  static const D3D11_BLEND destination[]={D3D11_BLEND_ZERO,D3D11_BLEND_ONE,D3D11_BLEND_SRC_COLOR,D3D11_BLEND_INV_SRC_COLOR,D3D11_BLEND_SRC_ALPHA,D3D11_BLEND_INV_SRC_ALPHA,D3D11_BLEND_DEST_ALPHA,D3D11_BLEND_INV_DEST_ALPHA};
  unsigned src=(mode>>8)&7,dst=(mode>>5)&7;
  target.SrcBlend=source[src];target.DestBlend=destination[dst];
  auto alphaFactor=[](D3D11_BLEND value){return value==D3D11_BLEND_DEST_COLOR?D3D11_BLEND_DEST_ALPHA:value==D3D11_BLEND_INV_DEST_COLOR?D3D11_BLEND_INV_DEST_ALPHA:value==D3D11_BLEND_SRC_COLOR?D3D11_BLEND_SRC_ALPHA:value==D3D11_BLEND_INV_SRC_COLOR?D3D11_BLEND_INV_SRC_ALPHA:value;};
  target.SrcBlendAlpha=alphaFactor(target.SrcBlend);target.DestBlendAlpha=alphaFactor(target.DestBlend);
  target.BlendOp=target.BlendOpAlpha=D3D11_BLEND_OP_ADD;
  if(mode&2048){target.BlendEnable=TRUE;target.SrcBlend=target.DestBlend=target.SrcBlendAlpha=target.DestBlendAlpha=D3D11_BLEND_ONE;target.BlendOp=target.BlendOpAlpha=D3D11_BLEND_OP_REV_SUBTRACT;}
  if(state.destinationAlpha().enabled){
   if(!target.BlendEnable){target.SrcBlend=D3D11_BLEND_ONE;target.DestBlend=D3D11_BLEND_ZERO;}
   auto originalAlpha=[](D3D11_BLEND factor){return factor==D3D11_BLEND_SRC_ALPHA?D3D11_BLEND_SRC1_ALPHA:factor==D3D11_BLEND_INV_SRC_ALPHA?D3D11_BLEND_INV_SRC1_ALPHA:factor;};
   target.SrcBlend=originalAlpha(target.SrcBlend);target.DestBlend=originalAlpha(target.DestBlend);
   target.BlendEnable=TRUE;target.SrcBlendAlpha=D3D11_BLEND_ONE;target.DestBlendAlpha=D3D11_BLEND_ZERO;target.BlendOpAlpha=D3D11_BLEND_OP_ADD;
  }
  Microsoft::WRL::ComPtr<ID3D11BlendState> result;check(device->CreateBlendState(&desc,&result));return result;
 }
 Microsoft::WRL::ComPtr<ID3D11RasterizerState> createCullMode(const GxCommandState& state){
  unsigned mode=(state.bp(0)>>14)&3;
  D3D11_RASTERIZER_DESC desc{};desc.FillMode=D3D11_FILL_SOLID;
  desc.CullMode=mode==1?D3D11_CULL_BACK:mode==2?D3D11_CULL_FRONT:D3D11_CULL_NONE;
  // D3D clockwise front convention; GX hardware cull values map 1=back, 2=front.
  desc.FrontCounterClockwise=FALSE;desc.DepthClipEnable=TRUE;desc.ScissorEnable=TRUE;
  Microsoft::WRL::ComPtr<ID3D11RasterizerState> result;
  check(device->CreateRasterizerState(&desc,&result));return result;
 }
 std::array<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>,8> materialViews;
 std::array<Microsoft::WRL::ComPtr<ID3D11SamplerState>,8> materialSamplers;
 Microsoft::WRL::ComPtr<ID3D11DepthStencilState> createDepthMode(const GxCommandState& state){
  unsigned mode=state.bp(0x40);
  D3D11_DEPTH_STENCIL_DESC desc{};desc.DepthEnable=(mode&1)!=0;
  desc.DepthWriteMask=(mode&16)?D3D11_DEPTH_WRITE_MASK_ALL:D3D11_DEPTH_WRITE_MASK_ZERO;
  desc.DepthFunc=D3D11_COMPARISON_FUNC(((mode>>1)&7)+1);
  Microsoft::WRL::ComPtr<ID3D11DepthStencilState> result;
  check(device->CreateDepthStencilState(&desc,&result));return result;
 }
 static void check(HRESULT result){if(FAILED(result))throw std::runtime_error("Native model draw Direct3D failure");}
 D3D11_RECT authoredScissor{};bool hasScissor=false;
 D3D11_VIEWPORT authoredViewport{};bool hasViewport=false;
 Microsoft::WRL::ComPtr<ID3D11Device> device;Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
 Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;Microsoft::WRL::ComPtr<ID3D11InputLayout> layout;
 Microsoft::WRL::ComPtr<ID3D11Buffer> tevConstants;
 Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer;Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthState;
};
}











