#include <revolution/os/OSTime.h>
#ifdef nullptr
#undef nullptr
#endif
#include <chrono>
#include <algorithm>
namespace {
using Clock=std::chrono::steady_clock;
const Clock::time_point origin=Clock::now();
constexpr u64 ticksPerSecond=60750000;
// Split seconds from the remainder to avoid multiplying a full nanosecond duration.
u64 ticks(){
 const auto ns=std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now()-origin).count();
 const u64 elapsed=u64(ns);
 return (elapsed/1000000000)*ticksPerSecond+(elapsed%1000000000)*ticksPerSecond/1000000000;
}
}
extern "C" OSTime OSGetTime(){return OSTime(ticks());}
extern "C" OSTick OSGetTick(){return OSTick(ticks());}
extern "C" { u32 __OSBusClock=243000000; }
#include "native_thread_cancellation.hpp"
#include <thread>
namespace {
bool isGregorianLeapYear(int year){return year%4==0&&(year%100!=0||year%400==0);}
int daysBeforeYear(int year){if(year<1)return 0;return (year+3)/4-(year-1)/100+(year-1)/400;}
void setCalendarDate(int64_t absoluteDay,OSCalendarTime* calendar){
 static constexpr int ordinaryMonthDays[]={0,31,59,90,120,151,181,212,243,273,304,334};
 static constexpr int leapMonthDays[]={0,31,60,91,121,152,182,213,244,274,305,335};
 calendar->wday=int((absoluteDay+6)%7);
 int year=int(absoluteDay/365);
 int yearStart;
 while(absoluteDay<(yearStart=daysBeforeYear(year)+365*year))--year;
 absoluteDay-=yearStart;
 calendar->year=year;
 calendar->yday=int(absoluteDay);
 const int* monthDays=isGregorianLeapYear(year)?leapMonthDays:ordinaryMonthDays;
 int month=11;
 while(month>0&&absoluteDay<monthDays[month])--month;
 calendar->mon=month;
 calendar->mday=int(absoluteDay-monthDays[month])+1;
}
}
extern "C" void OSSleepTicks(OSTime duration){
 nativeThreadCancellationPoint();
 if(duration<=0){std::this_thread::yield();return;}
 const u64 start=ticks();const u64 requested=u64(duration);
 while(true){nativeThreadCancellationPoint();const u64 elapsed=ticks()-start;if(elapsed>=requested)return;const u64 slice=std::min<u64>(requested-elapsed,607500);const u64 ns=(slice*1000000000+ticksPerSecond-1)/ticksPerSecond;std::this_thread::sleep_for(std::chrono::nanoseconds(ns));}
}
extern "C" void OSTicksToCalendarTime(OSTime value,OSCalendarTime* calendar){
 if(!calendar)return;
 int64_t wholeSeconds=value/int64_t(ticksPerSecond);
 int64_t fraction=value%int64_t(ticksPerSecond);
 if(fraction<0){fraction+=ticksPerSecond;--wholeSeconds;}
 calendar->usec=int(((fraction*8)/(ticksPerSecond/125000))%1000);
 calendar->msec=int((fraction/(ticksPerSecond/1000))%1000);
 int64_t day=wholeSeconds/86400+0xB2575;
 int secondsInDay=int(wholeSeconds%86400);
 if(secondsInDay<0){--day;secondsInDay+=86400;}
 setCalendarDate(day,calendar);
 calendar->hour=secondsInDay/3600;
 calendar->min=(secondsInDay/60)%60;
 calendar->sec=secondsInDay%60;
}


