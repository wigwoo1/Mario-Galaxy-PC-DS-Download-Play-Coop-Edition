#include <revolution/os/OSInterrupt.h>
#include <revolution/os/OSThread.h>
#include <mutex>
#include <stdexcept>
#include <condition_variable>
#include <unordered_map>
#include <memory>
#include <chrono>
#include "native_thread_cancellation.hpp"

// A cooperative host gate for SDK critical sections. This does not suspend
// arbitrary Windows threads or emulate hardware interrupt delivery.
namespace {
std::recursive_mutex gate;
thread_local bool interruptsEnabled=true;
thread_local int schedulerDepth=0;
}
extern "C" BOOL OSDisableInterrupts(){
 const bool previous=interruptsEnabled;
 if(previous){gate.lock();interruptsEnabled=false;}
 return previous;
}
extern "C" BOOL OSRestoreInterrupts(BOOL enabled){
 const bool previous=interruptsEnabled;
 if(bool(enabled)!=previous){
  if(enabled){interruptsEnabled=true;gate.unlock();}
  else {gate.lock();interruptsEnabled=false;}
 }
 return previous;
}
extern "C" BOOL OSEnableInterrupts(){return OSRestoreInterrupts(true);}
extern "C" s32 OSDisableScheduler(){
 gate.lock();return schedulerDepth++;
}
extern "C" s32 OSEnableScheduler(){
 if(!schedulerDepth)throw std::logic_error("Unbalanced native scheduler enable");
 const int previous=schedulerDepth--;gate.unlock();return previous;
}

namespace {
struct ThreadWaitState {std::condition_variable_any changed;unsigned long long generation=0;};
std::unordered_map<OSThreadQueue*,std::shared_ptr<ThreadWaitState>> waitQueues;
}
extern "C" void OSInitThreadQueue(OSThreadQueue* queue){if(!queue)throw std::invalid_argument("Null thread queue");std::lock_guard<std::recursive_mutex> lock(gate);*queue={};waitQueues[queue]=std::make_shared<ThreadWaitState>();}
extern "C" void OSSleepThread(OSThreadQueue* queue){
 const BOOL previous=OSDisableInterrupts();
 try {
  auto found=waitQueues.find(queue);if(found==waitQueues.end())throw std::logic_error("Uninitialized thread queue");
  if(schedulerDepth)throw std::logic_error("Cannot sleep with scheduler disabled");
  auto state=found->second;auto generation=state->generation;
  std::unique_lock<std::recursive_mutex> lock(gate,std::adopt_lock);
  try {while(state->generation==generation){nativeThreadCancellationPoint();state->changed.wait_for(lock,std::chrono::milliseconds(10));}nativeThreadCancellationPoint();}
  catch(...){lock.release();throw;}
  lock.release();
 }catch(...){OSRestoreInterrupts(previous);throw;}
 OSRestoreInterrupts(previous);
}
extern "C" void OSWakeupThread(OSThreadQueue* queue){std::lock_guard<std::recursive_mutex> lock(gate);auto found=waitQueues.find(queue);if(found==waitQueues.end())throw std::logic_error("Uninitialized thread queue");++found->second->generation;found->second->changed.notify_all();}
