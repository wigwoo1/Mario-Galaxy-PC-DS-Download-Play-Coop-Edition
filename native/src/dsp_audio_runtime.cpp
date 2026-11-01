// SPDX-License-Identifier: GPL-2.0-or-later
#include <Native/DspAudioRuntime.hpp>
#include <Native/DspAudioDispatch.hpp>
#include <Native/DspMailbox.hpp>
#include "native_jkr_aram_port.hpp"
#include <memory>
namespace {
struct Runtime {
    std::shared_ptr<NativeAramMemory> aram;
    Native::DspNativeAudioBridge bridge;
    Native::DspAudioDispatch dispatch;
    static bool read(void* context,u32 offset,void* output,std::size_t bytes){
        auto& runtime=*static_cast<Runtime*>(context);
        return nativeReadBoundAramSamples(runtime.aram.get(),offset,output,bytes);
    }
    Runtime(std::shared_ptr<NativeAramMemory> memory,NativeDspRequestCallback request)
      :aram(std::move(memory)),bridge(Native::nativeDspAddressSpace(),read,this),dispatch(bridge,request){}
    static s32 process(void* context,const u32* words,u32 count,u16* token){return static_cast<Runtime*>(context)->dispatch.process(words,count,token);}
    static void service(void* context){static_cast<Runtime*>(context)->dispatch.service();}
};
// Match the existing host AI provider's process-lifetime coordinator. Normal
// stop/reset frees the owned renderer; the tiny coordinator avoids static
// destruction order racing other SDK globals at DLL/process shutdown.
std::unique_ptr<Runtime>& state(){static auto* owner=new std::unique_ptr<Runtime>;return *owner;}
bool start(NativeDspRequestCallback request){
    nativeStopDspAudioRuntime();
    auto aram=nativeAcquireAramTransport();
    if(!request||!aram)return false;
    auto runtime=std::make_unique<Runtime>(std::move(aram),request);
    nativeSetDspHostAudioBackend(Runtime::process,Runtime::service,runtime.get());
    state()=std::move(runtime);return true;
}
}
void nativeConfigureDspAudioRuntime(){nativeSetDspAudioBootHooks(start,nativeStopDspAudioRuntime);}
void nativeStopDspAudioRuntime(){nativeSetDspHostAudioBackend(nullptr,nullptr,nullptr);state().reset();}
bool nativeDspAudioRuntimeActive(){return bool(state());}
bool nativeDspAudioRuntimeFailed(){return state()&&state()->dispatch.failed();}
