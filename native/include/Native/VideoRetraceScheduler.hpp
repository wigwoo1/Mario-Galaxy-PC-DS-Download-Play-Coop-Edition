#pragma once
#include <Native/VideoInterface.hpp>
#include <thread>
#include <exception>
namespace Native {
// Owns the host timing thread; stop/join before destroying original callback owners.
class VideoRetraceScheduler {
public:
 VideoRetraceScheduler(VideoInterface& video,std::chrono::nanoseconds period):video(video),period(period){
  if(period.count()<=0)throw std::invalid_argument("Invalid retrace period");
  worker=std::thread([this]{run();});
 }
 ~VideoRetraceScheduler(){stop();}
 // Callbacks may request shutdown. The owning host thread performs the join.
 void requestStop(){
  {std::lock_guard<std::mutex> lock(mutex);stopping=true;}wake.notify_all();
 }
 void stop(){
  requestStop();
  if(worker.joinable()){
   if(worker.get_id()==std::this_thread::get_id())
    throw std::logic_error("Retrace callback must request stop; owner must join");
   worker.join();
  }
 }
 void checkFailure()const{std::lock_guard<std::mutex> lock(mutex);if(failure)std::rethrow_exception(failure);}
 VideoRetraceScheduler(const VideoRetraceScheduler&)=delete;
 VideoRetraceScheduler& operator=(const VideoRetraceScheduler&)=delete;
private:
 void run()noexcept{
  try{
   auto next=std::chrono::steady_clock::now()+period;
   while(true){
    {std::unique_lock<std::mutex> lock(mutex);if(wake.wait_until(lock,next,[&]{return stopping;}))return;}
    video.retrace();
    next+=period;
    // Do not dispatch a burst of stale interrupts after a blocked host callback.
    auto now=std::chrono::steady_clock::now();if(next<=now)next=now+period;
   }
  }catch(...){std::lock_guard<std::mutex> lock(mutex);failure=std::current_exception();stopping=true;}
 }
 VideoInterface& video;std::chrono::nanoseconds period;mutable std::mutex mutex;
 std::condition_variable wake;bool stopping=false;std::exception_ptr failure;std::thread worker;
};
}

