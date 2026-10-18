#include <JSystem/JAudio2/JASProbe.hpp>
#include <revolution/os.h>
#include <cassert>
#include <cmath>
#include <cstdio>
extern "C" { u32 __OSBusClock=243000000; }
static OSTime timeValue;
extern "C" OSTime OSGetTime(){return timeValue;}
int main(){JASProbe p{};JASProbe::sProbeTable[5]=&p;timeValue=0xfffffff0;JASProbe::start(5,"mix");timeValue+=1000;JASProbe::stop(5);float expected=1000/(OS_TIMER_CLOCK/59.94f);assert(std::fabs(p._8-expected)<1e-8f);assert(p._1A8==1);for(unsigned i=1;i<105;++i){timeValue+=100; p.start("mix");timeValue+=1000;p.stop();}assert(p._1A8==105);assert(std::fabs(p._14-expected*100)<.0001f);assert(p._10==p._8);auto count=p._1A8;JASProbe::start(-1,"invalid");JASProbe::stop(-1);JASProbe::start(16,"invalid");JASProbe::stop(16);JASProbe::start(4,"empty");JASProbe::stop(4);assert(p._1A8==count);puts("PASS original audio profiling wraparound, rolling history, maximum and guarded dispatch (deterministic clock).");}

