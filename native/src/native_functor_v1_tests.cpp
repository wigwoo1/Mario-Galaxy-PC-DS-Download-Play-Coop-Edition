#include <Game/Util/Functor.hpp>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <memory>
static JKRHeap* expectedHeap;static unsigned allocs=0,frees=0;
void* JKRHeap::alloc(u32 size,int alignment,JKRHeap* heap){assert(heap==expectedHeap&&alignment==alignof(MR::NativeCallbackAllocation));++allocs;return std::malloc(size);}
void JKRHeap::free(void* bytes,JKRHeap* heap){assert(heap==expectedHeap);++frees;std::free(bytes);}
struct Receiver {bool mario=false;mutable int observed=0;void setPlayer(bool value){mario=value;}void inspect(int* value)const{observed=*value;}};
int main(){
 Receiver receiver;bool argument=true;auto callback=MR::Functor(&receiver,&Receiver::setPlayer,argument);argument=false;
 callback();assert(receiver.mario);receiver.mario=false;
 {std::unique_ptr<MR::FunctorBase> cloned(callback.clone(nullptr));(*cloned)();assert(receiver.mario);}
 expectedHeap=reinterpret_cast<JKRHeap*>(uintptr_t(1));
 {std::unique_ptr<MR::FunctorBase> cloned(callback.clone(expectedHeap));receiver.mario=false;(*cloned)();assert(receiver.mario);
  std::unique_ptr<MR::FunctorBase> again(cloned->clone(expectedHeap));receiver.mario=false;(*again)();assert(receiver.mario);}
 assert(allocs==2&&frees==2);
 const Receiver& constant=receiver;int value=7;auto observe=MR::Functor(&constant,&Receiver::inspect,&value);
 value=9;{std::unique_ptr<MR::FunctorBase> cloned(observe.clone(expectedHeap));(*cloned)();assert(receiver.observed==9);}
 assert(allocs==3&&frees==3);
 std::puts("PASS single-argument member callback: bool value capture, const pointer capture, ordinary/JKR cloning and owned destruction");
}
