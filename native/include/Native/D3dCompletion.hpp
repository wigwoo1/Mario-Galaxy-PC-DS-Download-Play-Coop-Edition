#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <atomic>
namespace Native {
class D3dFrameAborted final:public std::exception {
public:const char* what()const noexcept override{return "GX frame aborted by host watchdog";}
};
inline void waitForD3dCompletion(ID3D11DeviceContext& context,std::chrono::milliseconds timeout,const std::atomic<bool>* abortRequested=nullptr){
 if(abortRequested&&abortRequested->load(std::memory_order_acquire))throw D3dFrameAborted();
 Microsoft::WRL::ComPtr<ID3D11Device> device;context.GetDevice(&device);
 D3D11_QUERY_DESC description{D3D11_QUERY_EVENT,0};Microsoft::WRL::ComPtr<ID3D11Query> query;
 if(FAILED(device->CreateQuery(&description,&query)))throw std::runtime_error("Unable to create GPU completion query");
 context.End(query.Get());context.Flush();
 auto deadline=std::chrono::steady_clock::now()+timeout;
 while(true){
  if(abortRequested&&abortRequested->load(std::memory_order_acquire))throw D3dFrameAborted();
  BOOL complete=FALSE;HRESULT status=context.GetData(query.Get(),&complete,sizeof(complete),D3D11_ASYNC_GETDATA_DONOTFLUSH);
  if(FAILED(status))throw std::runtime_error("GPU completion query failed");
  if(status==S_OK&&complete)return;
  if(std::chrono::steady_clock::now()>=deadline)throw std::runtime_error("GPU completion timed out");
  std::this_thread::yield();
 }
}
}
