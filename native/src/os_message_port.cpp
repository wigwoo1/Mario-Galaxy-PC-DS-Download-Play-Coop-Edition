#include "native_thread_cancellation.hpp"
#include <chrono>
#include <revolution/types.h>
#include <revolution/os/OSMessage.h>
#ifdef nullptr
#undef nullptr
#endif
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
namespace {
struct QueueState {std::mutex mutex;std::condition_variable readable,writable;};
std::mutex registryMutex;std::map<OSMessageQueue*,std::shared_ptr<QueueState>> registry;
std::shared_ptr<QueueState> state(OSMessageQueue* queue){std::lock_guard<std::mutex> lock(registryMutex);auto found=registry.find(queue);if(found==registry.end())throw std::logic_error("Uninitialized native message queue");return found->second;}
BOOL send(OSMessageQueue* queue,OSMessage message,s32 flags,bool front){auto host=state(queue);std::unique_lock<std::mutex> lock(host->mutex);
 while(queue->usedCount==queue->msgCount){if(!(flags&OS_MESSAGE_BLOCK))return 0;nativeThreadCancellationPoint();host->writable.wait_for(lock,std::chrono::milliseconds(10));nativeThreadCancellationPoint();}
 s32 index;if(front){queue->firstIndex=(queue->firstIndex+queue->msgCount-1)%queue->msgCount;index=queue->firstIndex;}else index=(queue->firstIndex+queue->usedCount)%queue->msgCount;
 queue->msgArray[index]=message;++queue->usedCount;lock.unlock();host->readable.notify_one();return 1;}
}
extern "C" void OSInitMessageQueue(OSMessageQueue* queue,OSMessage* storage,s32 count){if(!queue || !storage || count<=0)throw std::invalid_argument("Invalid native message queue storage");auto host=std::make_shared<QueueState>();std::lock_guard<std::mutex> lock(registryMutex);queue->queueSend={};queue->queueReceive={};queue->msgArray=storage;queue->msgCount=count;queue->firstIndex=0;queue->usedCount=0;registry[queue]=std::move(host);}
extern "C" BOOL OSSendMessage(OSMessageQueue* queue,OSMessage message,s32 flags){return send(queue,message,flags,false);}
extern "C" BOOL OSJamMessage(OSMessageQueue* queue,OSMessage message,s32 flags){return send(queue,message,flags,true);}
extern "C" BOOL OSReceiveMessage(OSMessageQueue* queue,OSMessage* out,s32 flags){auto host=state(queue);std::unique_lock<std::mutex> lock(host->mutex);
 while(!queue->usedCount){if(!(flags&OS_MESSAGE_BLOCK))return 0;nativeThreadCancellationPoint();host->readable.wait_for(lock,std::chrono::milliseconds(10));nativeThreadCancellationPoint();}
 if(out)*out=queue->msgArray[queue->firstIndex];queue->firstIndex=(queue->firstIndex+1)%queue->msgCount;--queue->usedCount;lock.unlock();host->writable.notify_one();return 1;}



