#include "JSystem/JAudio2/JASCallback.hpp"
#include <revolution/os/OSInterrupt.h>
#include <cassert>
#include <cstdio>
struct State { int calls=0; s32 result=0; };
s32 run(void* arg) { auto& s=*static_cast<State*>(arg); ++s.calls; return s.result; }
int main() {
 JASCallbackMgr mgr; State keep, once; once.result=-1;
 assert(mgr.regist(run,&keep)); assert(mgr.regist(run,&once));
 mgr.callback(); mgr.callback(); assert(keep.calls==2 && once.calls==1);
 assert(mgr.reject(run,&once)==0); assert(mgr.regist(run,&keep));
 assert(mgr.reject(run,&keep)==2); mgr.callback(); assert(keep.calls==2);
 State slots[32]; for(auto& s:slots) assert(mgr.regist(run,&s));
 assert(!mgr.regist(run,&keep)); assert(mgr.reject(run,&slots[15])==1);
 assert(mgr.regist(run,&keep)); mgr.callback(); assert(keep.calls==3);
 for(int i=0;i<32;++i) assert(slots[i].calls==(i==15?0:1));
 assert(OSDisableInterrupts()); assert(!OSDisableInterrupts());
 assert(mgr.reject(run,&keep)==1); assert(!OSRestoreInterrupts(true));
 assert(OSDisableInterrupts()); OSRestoreInterrupts(true);
 puts("Original audio callback registration, capacity, retention, removal and nested critical-section tests passed.");
}
