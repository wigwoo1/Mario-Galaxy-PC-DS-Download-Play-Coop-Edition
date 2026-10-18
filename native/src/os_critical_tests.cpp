#include <revolution/os/OSInterrupt.h>
#include <revolution/os/OSThread.h>
#include <cassert>
#include <thread>
#include <stdexcept>
#include <cstdio>
int main(){
 assert(OSDisableInterrupts()==1);
 assert(OSDisableInterrupts()==0);
 assert(OSRestoreInterrupts(0)==0);
 assert(OSDisableScheduler()==0 && OSDisableScheduler()==1);
 assert(OSEnableScheduler()==2 && OSEnableScheduler()==1);
 assert(OSRestoreInterrupts(1)==0 && OSEnableInterrupts()==1);
 assert(OSRestoreInterrupts(0)==1 && OSEnableInterrupts()==0);
 bool rejected=false;try{OSEnableScheduler();}catch(const std::logic_error&){rejected=true;}assert(rejected);
 unsigned shared=0;
 auto worker=[&]{for(unsigned i=0;i<2000;++i){auto state=OSDisableInterrupts();auto old=shared;std::this_thread::yield();shared=old+1;assert(OSRestoreInterrupts(state)==0);}};
 std::thread first(worker),second(worker);first.join();second.join();assert(shared==4000);
 shared=0;
 auto scheduled=[&]{for(unsigned i=0;i<2000;++i){assert(OSDisableScheduler()==0);auto old=shared;std::this_thread::yield();shared=old+1;assert(OSEnableScheduler()==1);}};
 std::thread third(scheduled),fourth(scheduled);third.join();fourth.join();assert(shared==4000);
 std::puts("PASS native SDK critical sections: nested state, scheduler counts, two-thread exclusion");
}
