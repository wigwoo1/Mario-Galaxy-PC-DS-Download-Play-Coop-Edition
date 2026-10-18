#include <revolution/os/OSTime.h>
#include <chrono>
#include <thread>
#include <cassert>
#include <cstdio>
#include <revolution/os/OSThread.h>
#include "native_thread_cancellation.hpp"
int main(){
 OSCalendarTime calendar{};
 OSTicksToCalendarTime(0,&calendar);
 assert(calendar.year==2000&&calendar.mon==0&&calendar.mday==1&&calendar.wday==6&&calendar.yday==0);
 constexpr int64_t ticksPerDay=int64_t(86400)*60750000;
 OSTicksToCalendarTime(int64_t(7305+59)*ticksPerDay,&calendar);
 assert(calendar.year==2020&&calendar.mon==1&&calendar.mday==29&&calendar.yday==59&&calendar.wday==6);
 OSTicksToCalendarTime(int64_t(36525+59)*ticksPerDay,&calendar);
 assert(calendar.year==2100&&calendar.mon==2&&calendar.mday==1&&calendar.wday==1);
 OSTicksToCalendarTime(-1,&calendar);
 assert(calendar.year==1999&&calendar.mon==11&&calendar.mday==31&&calendar.wday==5&&calendar.hour==23&&calendar.min==59&&calendar.sec==59&&calendar.msec==999&&calendar.usec==999);
 OSTicksToCalendarTime(60750000+7499952,&calendar);
 assert(calendar.sec==1&&calendar.msec==123&&calendar.usec==456);
 const auto hostStart=std::chrono::steady_clock::now();const auto start=OSGetTime();
 std::this_thread::sleep_for(std::chrono::milliseconds(30));
 const auto end=OSGetTime();const auto hostEnd=std::chrono::steady_clock::now();
 const double actual=double(end-start)/60750000;
 const double host=std::chrono::duration<double>(hostEnd-hostStart).count();
 assert(end>start&&actual>.02&&actual<host+.005&&actual>host-.005);
 for(int i=0;i<10000;++i){const auto before=OSGetTime();const auto tick=OSGetTick();const auto after=OSGetTime();assert(after>=before);assert(u32(tick-u32(before))<=u64(after-before));}
 const auto sleepStart=OSGetTime();OSSleepTicks(1215000);assert(OSGetTime()-sleepStart>=1215000);OSSleepTicks(0);OSSleepTicks(-1);std::atomic<bool> cancelled{false};nativeThreadCancellation=&cancelled;std::thread canceller([&]{std::this_thread::sleep_for(std::chrono::milliseconds(5));cancelled=true;});bool unwound=false;try{OSSleepTicks(60750000);}catch(const NativeThreadCancelled&){unwound=true;}nativeThreadCancellation=nullptr;canceller.join();assert(unwound);
 std::puts("PASS native SDK timer: Wii tick rate, monotonic time and shared 32/64-bit timebase");
}

