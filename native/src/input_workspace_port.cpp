#include <atomic>
#include <Native/InputWorkspace.hpp>
#include <JSystem/JKernel/JKRHeap.hpp>
extern "C" void OSPanic(const char*,int,const char*,...);
namespace {std::atomic<uint32_t> budget{0};}
bool nativeConfigureInputWorkspace(uint32_t bytes){
    if(JKRHeap::sRootHeap || bytes<4096 || bytes>16*1024*1024)return false;
    budget.store(bytes);return true;
}
uint32_t nativeInputWorkspaceBytes(){
    auto bytes=budget.load();
    if(!bytes)OSPanic(__FILE__,__LINE__,"Native input workspace budget not configured");
    return bytes;
}
