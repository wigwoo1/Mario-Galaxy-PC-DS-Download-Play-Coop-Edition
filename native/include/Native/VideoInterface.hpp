#pragma once
#include <revolution/vi.h>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <stdexcept>
namespace Native {
// Host scheduler supplies retrace events. This core does not simulate presentation.
class VideoInterface {
public:
 struct State { GXRenderModeObj mode{}; bool configured=false,black=true;void* framebuffer=nullptr; };
 State current()const {std::lock_guard<std::mutex> lock(mutex);return active;}
 void configure(const GXRenderModeObj& mode){
  if(!mode.fbWidth||!mode.efbHeight||!mode.xfbHeight||!mode.viWidth||!mode.viHeight)
   throw std::invalid_argument("Empty VI render dimensions");
  std::lock_guard<std::mutex> lock(mutex);pending.mode=mode;pending.configured=true;
  // Petari's VI idle threshold is 15,000 retraces for PAL and 18,000 otherwise.
  dimmingTimeout=((mode.viTVmode>>2)==1u)?15000u:18000u;idleRetraces=0;
 }
 void setBlack(bool black){std::lock_guard<std::mutex> lock(mutex);pending.black=black;}
 void setNextFramebuffer(void* buffer){std::lock_guard<std::mutex> lock(mutex);pending.framebuffer=buffer;}
 void* nextFramebuffer()const{std::lock_guard<std::mutex> lock(mutex);return flushed.framebuffer;}
 void flush(){std::lock_guard<std::mutex> lock(mutex);flushed=pending;hasFlush=true;}
 VIRetraceCallback setPre(VIRetraceCallback callback){std::lock_guard<std::mutex> lock(mutex);auto old=pre;pre=callback;return old;}
 VIRetraceCallback setPost(VIRetraceCallback callback){std::lock_guard<std::mutex> lock(mutex);auto old=post;post=callback;return old;}
 u32 retraceCount()const{std::lock_guard<std::mutex> lock(mutex);return count;}
 BOOL setDimmingEnabled(BOOL enabled){if(enabled!=0&&enabled!=1)throw std::invalid_argument("Invalid VI dimming flag");std::lock_guard<std::mutex> lock(mutex);const BOOL old=dimmingEnabled?1:0;dimmingEnabled=enabled!=0;return old;}
 u32 dimmingCount()const{std::lock_guard<std::mutex> lock(mutex);return idleRetraces>=dimmingTimeout?0:dimmingTimeout-idleRetraces;}
 void notifyActivity(){std::lock_guard<std::mutex> lock(mutex);idleRetraces=0;}
 bool waitForRetrace(std::chrono::milliseconds timeout){
  std::unique_lock<std::mutex> lock(mutex);const u32 observed=count;
  return retraced.wait_for(lock,timeout,[&]{return count!=observed&&!inRetrace;});
 }
 void retrace(){
  VIRetraceCallback before,after;u32 serial;
  {std::lock_guard<std::mutex> lock(mutex);
   if(inRetrace)throw std::logic_error("Concurrent or recursive VI retrace");
   inRetrace=true;serial=++count;if(dimmingEnabled&&idleRetraces<0xFFFFFFFFu)++idleRetraces;before=pre;
  }
  try{
   if(before)before(serial);
   {std::lock_guard<std::mutex> lock(mutex);if(hasFlush){active=flushed;hasFlush=false;}after=post;}
   if(after)after(serial);
  }catch(...){std::lock_guard<std::mutex> lock(mutex);inRetrace=false;retraced.notify_all();throw;}
  std::lock_guard<std::mutex> lock(mutex);inRetrace=false;retraced.notify_all();
 }
private:
 mutable std::mutex mutex;std::condition_variable retraced;State pending,flushed,active;bool hasFlush=false,inRetrace=false,dimmingEnabled=true;u32 count=0,idleRetraces=0,dimmingTimeout=18000;VIRetraceCallback pre=nullptr,post=nullptr;
};
}

