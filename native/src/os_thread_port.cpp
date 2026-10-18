#include "native_thread_cancellation.hpp"
#include <revolution/types.h>
#include <revolution/os/OSThread.h>
#ifdef nullptr
#undef nullptr
#endif
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
void nativeAssignCurrentThread(OSThread*);
namespace {
struct NativeThreadExited {void* result;};
struct ThreadState {std::atomic<bool> cancelled{false};std::mutex mutex;std::condition_variable ready;std::thread worker;bool done=false,detached=false,joining=false;void* result=nullptr;};
std::mutex registryMutex;std::map<OSThread*,std::shared_ptr<ThreadState>> registry;
std::shared_ptr<ThreadState> find(OSThread* thread){std::lock_guard<std::mutex> lock(registryMutex);auto item=registry.find(thread);return item==registry.end()?nullptr:item->second;}
}
void suspensionPoint(){auto* thread=OSGetCurrentThread();auto host=find(thread);if(!host)return;std::unique_lock<std::mutex> lock(host->mutex);if(thread->suspend>0&&!host->done){thread->state=OS_THREAD_STATE_READY;host->ready.wait(lock,[&]{return thread->suspend==0||host->cancelled.load();});if(host->cancelled.load())throw NativeThreadCancelled{};thread->state=OS_THREAD_STATE_RUNNING;}}
extern "C" BOOL OSCreateThread(OSThread* thread,void* (*entry)(void*),void* argument,void* stack,u32 stackSize,OSPriority priority,u16 attributes){
 if(!thread || !entry || priority<0 || priority>31 || !stack || !stackSize)return 0;
 auto host=std::make_shared<ThreadState>();host->detached=bool(attributes&1);
 {std::lock_guard<std::mutex> lock(registryMutex);if(registry.count(thread))return 0;*thread={};thread->state=OS_THREAD_STATE_READY;thread->attr=attributes;thread->suspend=1;thread->priority=priority;thread->base=priority;registry.emplace(thread,host);}
 try{host->worker=std::thread([=]{nativeAssignCurrentThread(thread);nativeThreadCancellation=&host->cancelled;nativeThreadSuspensionPoint=suspensionPoint;{std::unique_lock<std::mutex> lock(host->mutex);host->ready.wait(lock,[&]{return thread->suspend==0 || host->cancelled.load();});thread->state=OS_THREAD_STATE_RUNNING;}void* result=nullptr;try{nativeThreadCancellationPoint();result=entry(argument);}catch(const NativeThreadExited& exited){result=exited.result;}catch(const NativeThreadCancelled&){}nativeThreadSuspensionPoint=nullptr;nativeThreadCancellation=nullptr;{std::lock_guard<std::mutex> lock(host->mutex);host->result=result;thread->value=result;thread->state=host->detached?0:OS_THREAD_STATE_MORIBUND;host->done=true;if(host->detached){std::lock_guard<std::mutex> registryLock(registryMutex);registry.erase(thread);}}host->ready.notify_all();});if(host->detached)host->worker.detach();}
 catch(...){std::lock_guard<std::mutex> lock(registryMutex);registry.erase(thread);thread->state=0;return 0;}return 1;
}
extern "C" s32 OSResumeThread(OSThread* thread){auto host=find(thread);if(!host)return 0;std::lock_guard<std::mutex> lock(host->mutex);s32 previous=thread->suspend;if(thread->suspend>0)--thread->suspend;if(!thread->suspend)host->ready.notify_all();return previous;}
extern "C" BOOL OSJoinThread(OSThread* thread,void** result){auto host=find(thread);if(!host || thread==OSGetCurrentThread())return 0;{std::unique_lock<std::mutex> lock(host->mutex);if(host->detached || host->joining)return 0;host->joining=true;host->ready.wait(lock,[&]{return host->done;});if(result)*result=host->result;}host->worker.join();{std::lock_guard<std::mutex> lock(registryMutex);registry.erase(thread);}thread->state=0;return 1;}
extern "C" void OSYieldThread(){nativeThreadCancellationPoint();std::this_thread::yield();}
extern "C" BOOL OSIsThreadSuspended(OSThread* thread){if(!thread)return 0;auto host=find(thread);if(host){std::lock_guard<std::mutex> lock(host->mutex);return thread->suspend>0;}return thread->suspend>0;}
extern "C" BOOL OSIsThreadTerminated(OSThread* thread){if(!thread)return 1;auto host=find(thread);if(host){std::lock_guard<std::mutex> lock(host->mutex);return thread->state==OS_THREAD_STATE_MORIBUND || thread->state==0;}return thread->state==OS_THREAD_STATE_MORIBUND || thread->state==0;}

extern "C" BOOL OSSetThreadPriority(OSThread* thread, OSPriority priority){
 if(!thread || priority<0 || priority>31)return 0;
 auto host=find(thread);
 if(host){std::lock_guard<std::mutex> lock(host->mutex);thread->base=priority;thread->priority=priority;}
 else {thread->base=priority;thread->priority=priority;}
 return 1;
}

extern "C" OSPriority OSGetThreadPriority(OSThread* thread){
 if(!thread)return -1;
 auto host=find(thread);
 if(host){std::lock_guard<std::mutex> lock(host->mutex);return thread->priority;}
 return thread->priority;
}

extern "C" void OSDetachThread(OSThread* thread){
 auto host=find(thread);if(!host)return;
 std::lock_guard<std::mutex> lock(host->mutex);
 // Joining and detaching the same host handle concurrently is unsupported.
 if(host->joining || host->detached)return;
 host->worker.detach();host->detached=true;thread->attr|=1;
 if(host->done){thread->state=0;std::lock_guard<std::mutex> registryLock(registryMutex);registry.erase(thread);}
 host->ready.notify_all();
}

extern "C" void OSCancelThread(OSThread* thread){
 auto host=find(thread);if(!host)return;
 if(thread==OSGetCurrentThread()){host->cancelled.store(true);nativeThreadCancellationPoint();}
 {std::unique_lock<std::mutex> lock(host->mutex);if(host->joining)throw std::logic_error("Concurrent thread join and cancellation");host->joining=true;host->cancelled.store(true,std::memory_order_release);host->ready.notify_all();host->ready.wait(lock,[&]{return host->done;});}
 if(!host->detached && host->worker.joinable())host->worker.join();
 {std::lock_guard<std::mutex> lock(registryMutex);registry.erase(thread);}thread->state=0;
}


extern "C" void OSExitThread(void* result){if(!nativeThreadCancellation)throw std::logic_error("OSExitThread outside native worker");throw NativeThreadExited{result};}


extern "C" s32 OSSuspendThread(OSThread* thread){auto host=find(thread);if(!host)return 0;s32 previous;{std::lock_guard<std::mutex> lock(host->mutex);if(host->done)return 0;previous=thread->suspend;if(previous==0x7fffffff)return previous;++thread->suspend;}if(thread==OSGetCurrentThread())suspensionPoint();return previous;}
