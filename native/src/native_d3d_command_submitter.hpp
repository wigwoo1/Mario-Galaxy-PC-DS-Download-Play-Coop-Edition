#pragma once
#include "native_d3d_render_stream.hpp"
#include "native_d3d_rgb565_copy.hpp"
#include "native_d3d_xfb_copy.hpp"
#include <Native/CommandSubmission.hpp>
#include <Native/D3dEfb.hpp>
#include <Native/EfbReadback.hpp>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <mutex>

class NativeD3dCommandSubmitter:public Native::CommandSubmitter {
 NativeD3dRenderStream renderer;
 NativeRenderStream stream;
 Native::D3dEfb& efb;
 Native::EfbReadbackBinding readbackBinding;
 Native::GpuResources& resources;
 NativeD3dXfbCopy xfbCopy;
 NativeD3dRgb565Copy textureCopy;
 std::function<void(const Native::GxCommandState::CopyOperation&,const std::uint8_t*,std::size_t)> displayFrameHandler;
 NativeD3dRenderStream::CopyHandler copyHandler;
 std::atomic<bool> abortRequested{false};
 std::mutex executionMutex;
 std::mutex stateMutex;
 std::condition_variable stateChanged;
 std::exception_ptr executionFailure;
 bool finished=false;
public:
 static std::uint32_t readEfbPixel(void* object,std::uint16_t x,std::uint16_t y){return static_cast<Native::D3dEfb*>(object)->readArgb(x,y);}
 NativeD3dCommandSubmitter(ID3D11DeviceContext& context,Native::D3dEfb& target,Native::GpuResources& storage,
  NativeD3dRenderStream::CopyHandler copies={},
  std::function<void(const Native::GxCommandState::CopyOperation&,const std::uint8_t*,std::size_t)> displayFrame={})
  :renderer(context),efb(target),readbackBinding(&target,&readEfbPixel),resources(storage),
   xfbCopy(target,storage,context),textureCopy(target,storage,context),displayFrameHandler(std::move(displayFrame)),copyHandler(std::move(copies)){
  if(!copyHandler)copyHandler=[this](const Native::GxCommandState::CopyOperation& operation){
   if(operation.display()){
    xfbCopy(operation);
    if(displayFrameHandler){
     if(!operation.height()||operation.strideBytes()>SIZE_MAX/operation.height())
      throw std::overflow_error("Native XFB presentation size overflow");
     const auto bytes=std::size_t(operation.strideBytes())*operation.height();
     displayFrameHandler(operation,resources.resolve(operation.destinationAddress(),bytes),bytes);
    }
   }else textureCopy(operation);
  };
 }

 void submit(const std::uint8_t* data,std::size_t size,bool finish)override{
  // The D3D immediate context is thread-affine by policy. Alarm recovery may
  // submit from another host thread, so every batch is serialized here.
  std::unique_lock<std::mutex> executionLock(executionMutex);
  std::unique_lock<std::mutex> stateLock(stateMutex);
  if(executionFailure)std::rethrow_exception(executionFailure);
  if(abortRequested.exchange(false,std::memory_order_acq_rel))stream=NativeRenderStream{};
  auto candidate=stream;
  candidate.decode(data,size,resources);
  bool hasFinish=false;
  for(const auto& op:candidate.operations())hasFinish|=op.kind==NativeRenderOperation::Kind::Finish;
  if(finish&&!hasFinish)throw std::invalid_argument("GX finish submission has no finish command");
  auto batch=renderer.prepare(candidate.operations(),resources,copyHandler);
  finished=false;
  stream=std::move(candidate);
  stateLock.unlock();

  try{
   renderer.executePrepared(batch,efb,&abortRequested);
  }catch(const Native::D3dFrameAborted&){
   {
    std::lock_guard<std::mutex> lock(stateMutex);
    stream=NativeRenderStream{};
    finished=false;
    abortRequested.store(false,std::memory_order_release);
   }
   stateChanged.notify_all();
   return;
  }catch(...){
   {
    std::lock_guard<std::mutex> lock(stateMutex);
    executionFailure=std::current_exception();
   }
   stateChanged.notify_all();
   return;
  }

  if(hasFinish){
   {
    std::lock_guard<std::mutex> lock(stateMutex);
    finished=true;
   }
   stateChanged.notify_all();
  }
 }

 void waitForFinish()override{
  std::unique_lock<std::mutex> lock(stateMutex);
  if(!stateChanged.wait_for(lock,std::chrono::seconds(5),[this]{return finished||bool(executionFailure);} ))
   throw std::runtime_error("Timed out waiting for the GX recovery finish fence");
  if(executionFailure)std::rethrow_exception(executionFailure);
 }

 void abortFrame()noexcept override{
  // The watchdog never calls D3D from its scheduler thread. The render path
  // observes this signal while polling a fence or between decoded operations.
  abortRequested.store(true,std::memory_order_release);
 }
};
