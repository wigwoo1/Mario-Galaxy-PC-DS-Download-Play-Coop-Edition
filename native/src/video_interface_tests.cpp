#include <thread>
#include <Native/VideoInterface.hpp>
#include <cassert>
#include <cstdio>
Native::VideoInterface* vi;int stages=0;int first,second;
void before(u32 count){assert(count==1&&vi->retraceCount()==1);assert(vi->current().black);vi->setNextFramebuffer(&second);vi->setBlack(false);vi->flush();stages=1;}
void after(u32 count){assert(count==1&&stages==1);auto s=vi->current();assert(!s.black&&s.framebuffer==&second);stages=2;}
void recursive(u32){vi->retrace();}
int main(){
 Native::VideoInterface video;vi=&video;GXRenderModeObj mode{};mode.fbWidth=608;mode.efbHeight=448;mode.xfbHeight=448;mode.viWidth=640;mode.viHeight=448;
 video.configure(mode);mode.fbWidth=1;video.setNextFramebuffer(&first);video.flush();video.setNextFramebuffer(&second);
 assert(video.nextFramebuffer()==&first);assert(!video.current().configured&&video.current().framebuffer==nullptr);
 video.retrace();assert(video.current().configured&&video.current().mode.fbWidth==608&&video.current().framebuffer==&first);
 video.setNextFramebuffer(&first);video.setBlack(true);video.flush();video.retrace();
 // A new instance starts at count zero; callbacks can query and flush without a deadlock.
 Native::VideoInterface callbacks;vi=&callbacks;assert(callbacks.setPre(before)==nullptr&&callbacks.setPost(after)==nullptr);callbacks.retrace();assert(stages==2);
 assert(callbacks.setPre(recursive)==before);bool rejected=false;try{callbacks.retrace();}catch(const std::logic_error&){rejected=true;}assert(rejected);
 callbacks.setPre(nullptr);callbacks.setPost(nullptr);callbacks.retrace();assert(callbacks.retraceCount()==3);
 bool invalid=false;try{GXRenderModeObj empty{};video.configure(empty);}catch(const std::invalid_argument&){invalid=true;}assert(invalid&&video.current().mode.fbWidth==608);
 Native::VideoInterface waiting;
 assert(!waiting.waitForRetrace(std::chrono::milliseconds(1)));
 std::thread scheduler([&]{std::this_thread::sleep_for(std::chrono::milliseconds(30));waiting.retrace();});
 assert(waiting.waitForRetrace(std::chrono::milliseconds(1000)));scheduler.join();
 assert(waiting.retraceCount()==1);
 std::puts("PASS native VI state: pending/flush/latch, retained mode, callback reentry/order, exception recovery");
}

