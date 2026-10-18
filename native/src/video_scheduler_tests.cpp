#include <Native/VideoRetraceScheduler.hpp>
#include <cassert>
#include <cstdio>
void failCallback(u32){throw std::runtime_error("callback fixture failure");}
Native::VideoRetraceScheduler* callbackScheduler=nullptr;
void stopCallback(u32){callbackScheduler->requestStop();}
int main(){
 Native::VideoInterface video;
 {
  Native::VideoRetraceScheduler scheduler(video,std::chrono::milliseconds(10));
  for(int i=0;i<3;++i)assert(video.waitForRetrace(std::chrono::milliseconds(1000)));
  scheduler.checkFailure();scheduler.stop();
  const auto stopped=video.retraceCount();assert(stopped>=3);std::this_thread::sleep_for(std::chrono::milliseconds(30));assert(video.retraceCount()==stopped);
 }
 Native::VideoInterface failure;failure.setPre(failCallback);
 {Native::VideoRetraceScheduler scheduler(failure,std::chrono::milliseconds(5));
  assert(failure.waitForRetrace(std::chrono::milliseconds(1000)));scheduler.stop();
  bool caught=false;try{scheduler.checkFailure();}catch(const std::runtime_error&){caught=true;}assert(caught);
 }
 Native::VideoInterface selfStop;
 {Native::VideoRetraceScheduler scheduler(selfStop,std::chrono::milliseconds(10));
  callbackScheduler=&scheduler;selfStop.setPre(stopCallback);
  assert(selfStop.waitForRetrace(std::chrono::milliseconds(1000)));
  scheduler.stop();scheduler.checkFailure();auto count=selfStop.retraceCount();
  std::this_thread::sleep_for(std::chrono::milliseconds(20));assert(selfStop.retraceCount()==count);
 }
 bool rejected=false;try{Native::VideoRetraceScheduler scheduler(video,std::chrono::nanoseconds(0));}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
 std::puts("PASS timed native retraces: repeated waits, stop/join, callback failure propagation, invalid period");
}

