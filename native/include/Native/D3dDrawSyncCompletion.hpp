#pragma once
#include <Native/D3dCompletion.hpp>
#include <Native/DrawSyncCompletion.hpp>
#include <atomic>
namespace Native {
inline void completeD3dDrawSync(ID3D11DeviceContext& context,std::uint16_t token,std::chrono::milliseconds timeout,const std::atomic<bool>* abortRequested=nullptr){
 waitForD3dCompletion(context,timeout,abortRequested);
 dispatchDrawSyncCallback(token);
}
}
