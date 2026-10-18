#pragma once
#include "native_render_stream.hpp"
#include "native_model_projection.hpp"
#include "native_model_texture.hpp"
#include "gx_command_texture.hpp"
#include <Native/D3dDepthPeek.hpp>
#include <Native/D3dDrawSyncCompletion.hpp>
#include <Native/DrawCompletion.hpp>
#include <functional>
#include <memory>
#include <atomic>
#include <cstdio>
#include <cstdlib>
class NativeD3dRenderStream {
 struct Prepared {
  std::unique_ptr<Native::D3dModelDraw> draw;
  std::vector<Native::ProjectedVertex> vertices;
  std::array<std::unique_ptr<NativeModelTexture>,8> textures;std::uint64_t resourceGeneration=0;
 };
 Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
 Microsoft::WRL::ComPtr<ID3D11Device> device;
public:
 using CopyHandler=std::function<void(const Native::GxCommandState::CopyOperation&)>;
 explicit NativeD3dRenderStream(ID3D11DeviceContext& value):context(&value){value.GetDevice(&device);}
 struct Batch {std::vector<NativeRenderOperation> operations;std::vector<Prepared> prepared;CopyHandler copyHandler;const Native::GpuResources* resources;};
 Batch prepare(const std::vector<NativeRenderOperation>& operations,const Native::GpuResources& resources,const CopyHandler& copyHandler={}){
  Batch batch{operations,std::vector<Prepared>(operations.size()),copyHandler,&resources};auto& prepared=batch.prepared;
  // Validation/resource preparation happens before any Draw call or callback.
  for(size_t i=0;i<operations.size();++i){
   const auto& op=operations[i];
   if(op.kind==NativeRenderOperation::Kind::Copy&&!copyHandler)throw std::invalid_argument("Native EFB copy handler missing");
   if(op.kind!=NativeRenderOperation::Kind::Draw)continue;
   auto& item=prepared[i];item.resourceGeneration=resources.generation();item.draw=std::make_unique<Native::D3dModelDraw>(*context.Get());
   std::array<ID3D11ShaderResourceView*,8> views{};std::array<ID3D11SamplerState*,8> samplers{};
   std::array<std::array<unsigned,2>,8> textureDimensions{};
   std::array<std::array<unsigned,4>,8> textureFirstPixel{};
   auto usedTextures=Native::materialTextureSlots(op.state);
   for(unsigned slot=0;slot<8;++slot){
    if(usedTextures[slot]){
     auto image=decodeCommandTexture(op.state,slot,resources);
     textureDimensions[slot]={image.width,image.height};
     if(!image.pixels.empty())textureFirstPixel[slot]={image.pixels[0].r,image.pixels[0].g,image.pixels[0].b,image.pixels[0].a};
     item.textures[slot]=std::make_unique<NativeModelTexture>(*device.Get(),image);
     views[slot]=item.textures[slot]->view.Get();samplers[slot]=item.textures[slot]->sampler.Get();
    }
   }
   item.draw->setMaterial(op.state,views,samplers);item.draw->setDepthMode(op.state);item.draw->setBlendMode(op.state);item.draw->setCullMode(op.state);
   item.draw->setViewport(op.state);item.draw->setScissor(op.state);
   const bool rasterColor=gxTevConsumesRasterColor(op.state);
   const char* tracePath=std::getenv("GALAXY_RENDER_TRACE_FILE");
   static std::atomic<unsigned> tracedDrawCount{0};
   if(tracePath&&*tracePath){const unsigned traceIndex=tracedDrawCount.fetch_add(1,std::memory_order_relaxed);if(traceIndex<16){
    if(auto* trace=std::fopen(tracePath,"ab")){
     unsigned color0=0,color1=0,normal=0,tex0=0;
     for(const auto& vertex:op.vertices){color0+=vertex.hasColor[0];color1+=vertex.hasColor[1];normal+=vertex.hasNormal;tex0+=vertex.hasTexCoord0;}
     const auto colorRegister0=op.state.tevColor(0);
     std::fprintf(trace,"draw=%u vertices=%zu triangles=%zu stages=%u raster=%u attrs={color0:%u color1:%u normal:%u tex0:%u} firstColor={%u,%u,%u,%u} firstPosition={%.3f,%.3f,%.3f} tevC0={%d,%d,%d,%d}\n",traceIndex,op.vertices.size(),op.triangles.size(),op.state.tevStageCount(),unsigned(rasterColor),color0,color1,normal,tex0,op.vertices.empty()?0:op.vertices[0].colors[0][0],op.vertices.empty()?0:op.vertices[0].colors[0][1],op.vertices.empty()?0:op.vertices[0].colors[0][2],op.vertices.empty()?0:op.vertices[0].colors[0][3],op.vertices.empty()?0.f:op.vertices[0].position[0],op.vertices.empty()?0.f:op.vertices[0].position[1],op.vertices.empty()?0.f:op.vertices[0].position[2],colorRegister0[0],colorRegister0[1],colorRegister0[2],colorRegister0[3]);
     for(unsigned stage=0;stage<op.state.tevStageCount();++stage){const auto order=op.state.tevOrder(stage);const auto tev=op.state.tevStage(stage);const auto dims=order.enabled?textureDimensions[order.texture]:std::array<unsigned,2>{0,0};const auto sample=order.enabled?textureFirstPixel[order.texture]:std::array<unsigned,4>{0,0,0,0};std::fprintf(trace,"  tev[%u] tex=%u size=%ux%u sample={%u,%u,%u,%u} coord=%u channel=%u enabled=%u color=%u,%u,%u,%u alpha=%u,%u,%u,%u\n",stage,order.texture,dims[0],dims[1],sample[0],sample[1],sample[2],sample[3],order.coordinate,order.channel,unsigned(order.enabled),tev.color.inputs[0],tev.color.inputs[1],tev.color.inputs[2],tev.color.inputs[3],tev.alpha.inputs[0],tev.alpha.inputs[1],tev.alpha.inputs[2],tev.alpha.inputs[3]);}
     std::fclose(trace);
    }
   }}
   item.vertices=projectModelTriangles(op.vertices,op.triangles,op.state,{1,1,1,1},rasterColor);
  }
  return batch;
 }
 void executePrepared(Batch& batch,Native::D3dEfb& efb,const std::atomic<bool>* abortRequested=nullptr){
  const auto& operations=batch.operations;auto& prepared=batch.prepared;const auto& copyHandler=batch.copyHandler;
  Native::D3dDepthPeek depth(efb);Native::DepthPeekBinding depthBinding(depth);
  for(size_t i=0;i<operations.size();++i){if(abortRequested&&abortRequested->load(std::memory_order_acquire))throw Native::D3dFrameAborted();const auto& op=operations[i];
   switch(op.kind){
    case NativeRenderOperation::Kind::Draw:{
     auto& item=prepared[i];
     if(item.resourceGeneration!=batch.resources->generation()){
      std::array<ID3D11ShaderResourceView*,8> views{};std::array<ID3D11SamplerState*,8> samplers{};
      for(unsigned slot=0;slot<8;++slot)if(item.textures[slot]){
       auto image=decodeCommandTexture(op.state,slot,*batch.resources);
       item.textures[slot]=std::make_unique<NativeModelTexture>(*device.Get(),image);
       views[slot]=item.textures[slot]->view.Get();samplers[slot]=item.textures[slot]->sampler.Get();
      }
      item.draw->setMaterial(op.state,views,samplers);item.resourceGeneration=batch.resources->generation();
     }
     item.draw->draw(efb,item.vertices);break;
    }
    case NativeRenderOperation::Kind::Copy:copyHandler(op.copy);break;
    case NativeRenderOperation::Kind::Token:Native::completeD3dDrawSync(*context.Get(),op.token,std::chrono::milliseconds(2000),abortRequested);break;
    case NativeRenderOperation::Kind::Finish:Native::waitForD3dCompletion(*context.Get(),std::chrono::milliseconds(2000),abortRequested);Native::dispatchDrawDoneCallback();break;
   }
  }
 }
 void execute(const std::vector<NativeRenderOperation>& operations,Native::D3dEfb& efb,const Native::GpuResources& resources,const CopyHandler& copyHandler={}){auto batch=prepare(operations,resources,copyHandler);executePrepared(batch,efb);}
};


