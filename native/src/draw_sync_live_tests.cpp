#include "generated/DrawSyncLiveFixture.cpp"
#include <cassert>
#include <mutex>
#include <vector>
#include <cstdio>
static std::mutex eventsMutex;
static std::vector<void*> enabled;
static bool lastWasDisabled; static void* fifoWrite;
extern "C" void GXEnableBreakPt(void* pointer){std::lock_guard<std::mutex> lock(eventsMutex);enabled.push_back(pointer);lastWasDisabled=false;}
extern "C" void GXDisableBreakPt(){std::lock_guard<std::mutex> lock(eventsMutex);lastWasDisabled=true;}
extern "C" GXDrawSyncCallback GXSetDrawSyncCallback(GXDrawSyncCallback callback){return nullptr;}
extern "C" void GXFlush(){}
extern "C" GXBool GXGetCPUFifo(GXFifoObj*){return GX_TRUE;}
extern "C" void GXGetFifoPtrs(const GXFifoObj*,void** read,void** write){*read=nullptr;*write=fifoWrite;}
struct Callback:DrawSyncCallback {unsigned count=0;void drawSyncCallback(u16 token)override{assert(token==1);++count;}};
int main(){
 for(unsigned run=0;run<100;++run){
  auto* first=reinterpret_cast<void*>(uintptr_t(0x123456789abcdef0ull));
  auto* second=reinterpret_cast<void*>(uintptr_t(0x1234));
  {
   DrawSyncManager manager(4,16);
   nativeSendDrawMessage(&manager.mQueue,NativeDrawMessageKind::BreakPoint,first);
   nativeSendDrawMessage(&manager.mQueue,NativeDrawMessageKind::BreakPoint,second);
   nativeSendDrawMessage(&manager.mQueue,NativeDrawMessageKind::Token);
   nativeSendDrawMessage(&manager.mQueue,NativeDrawMessageKind::Token);
  } // Original adapted destructor sends shutdown and joins real native worker.
 }
  assert(enabled.size()==100&&lastWasDisabled);
 Callback callback;
 {
  DrawSyncManager manager(4,16);DrawSyncManager::sInstance=&manager;
  assert(manager.setCallback(2,1,&callback)==1);
  fifoWrite=reinterpret_cast<void*>(uintptr_t(0x100000001ull));manager.pushBreakPoint();
  fifoWrite=reinterpret_cast<void*>(uintptr_t(0x4321));manager.pushBreakPoint();
  manager.drawSyncCallbackSub(1);manager.drawSyncCallbackSub(1);
  DrawSyncManager::sInstance=nullptr;
 }
 assert(callback.count==2&&enabled.size()==101&&enabled.back()==reinterpret_cast<void*>(uintptr_t(0x4321))&&lastWasDisabled);
 // Pending breakpoint work during shutdown must leave GPU state disabled after join.
 for(unsigned i=0;i<100;++i){
  {DrawSyncManager manager(4,16);fifoWrite=reinterpret_cast<void*>(uintptr_t(0x100000001ull));manager.pushBreakPoint();manager.pushBreakPoint();}
  assert(lastWasDisabled);
 }
 for(size_t i=0;i<100;++i)assert(enabled[i]==reinterpret_cast<void*>(uintptr_t(0x1234)));
 std::puts("PASS native worker lifecycle, public breakpoint/callback dispatch, and pending-work shutdown ordering; GPU callbacks are fixtures");
}

