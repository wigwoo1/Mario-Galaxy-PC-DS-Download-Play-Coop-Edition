#include <chrono>
#include <thread>
#include <revolution/os/OSMutex.h>
#include <atomic>
#include <cassert>
#include <revolution/os/OSMessage.h>
OSMutex cancelMutex;OSMessageQueue cancelQueue;OSMessage cancelStorage;std::atomic<bool> cancelEntered{false},cancelUnwound{false};struct CancelGuard{~CancelGuard(){cancelUnwound=true;}};void* blockedEntry(void*){CancelGuard guard;cancelEntered=true;OSReceiveMessage(&cancelQueue,nullptr,OS_MESSAGE_BLOCK);assert(false);return nullptr;}
#include <atomic>
#include <cassert>
#include <cstdio>
#include <revolution/types.h>
#include <revolution/os/OSThread.h>
struct Work {OSThread* expected;std::atomic<int> calls{0};};
void* entry(void* raw){auto work=static_cast<Work*>(raw);assert(OSGetCurrentThread()==work->expected);assert(OSGetThreadPriority(OSGetCurrentThread())==7);assert(!OSJoinThread(OSGetCurrentThread(),nullptr));++work->calls;return raw;}
std::atomic<bool> exitUnwound{false};struct ExitGuard{~ExitGuard(){exitUnwound=true;}};void* exitEntry(void* raw){ExitGuard guard;OSExitThread(raw);assert(false);return nullptr;}
#ifdef nullptr
#undef nullptr
#endif
#include <chrono>
#include <thread>
std::atomic<bool> sleepEntered{false},sleepUnwound{false};struct SleepGuard{~SleepGuard(){sleepUnwound=true;}};void* sleepEntry(void*){SleepGuard guard;sleepEntered=true;OSSleepTicks(607500000);assert(false);return nullptr;}
std::atomic<bool> suspensionEntered{false},suspensionReturned{false};void* suspendEntry(void* raw){suspensionEntered=true;assert(OSSuspendThread(OSGetCurrentThread())==0);suspensionReturned=true;return raw;}
int main(){OSThread thread;char stack[4096];Work work{&thread};assert(OSCreateThread(&thread,entry,&work,stack+sizeof(stack),sizeof(stack),7,0));assert(work.calls==0 && thread.suspend==1);assert(OSIsThreadSuspended(&thread));assert(!OSIsThreadTerminated(&thread));assert(!OSCreateThread(&thread,entry,&work,stack+sizeof(stack),sizeof(stack),7,0));assert(OSResumeThread(&thread)==1);void* result=nullptr;assert(OSJoinThread(&thread,&result));assert(result==&work && work.calls==1 && thread.state==0);assert(OSIsThreadTerminated(&thread));assert(!OSIsThreadSuspended(&thread));assert(!OSJoinThread(&thread,nullptr));assert(OSCreateThread(&thread,entry,&work,stack+sizeof(stack),sizeof(stack),7,0));assert(OSResumeThread(&thread)==1);while(!OSIsThreadTerminated(&thread))OSYieldThread();assert(thread.state==OS_THREAD_STATE_MORIBUND);assert(!OSIsThreadSuspended(&thread));assert(OSJoinThread(&thread,nullptr));assert(work.calls==2);for(int mode=0;mode<3;++mode){
 assert(OSCreateThread(&thread,entry,&work,stack+sizeof(stack),sizeof(stack),7,mode==0?1:0));
 if(mode==1)OSDetachThread(&thread);
 assert(OSResumeThread(&thread)==1);
 while(!OSIsThreadTerminated(&thread))OSYieldThread();
 if(mode==2)OSDetachThread(&thread);
 assert(!OSJoinThread(&thread,nullptr));
 // The next iteration verifies released registry state and SDK storage reuse.
}
assert(work.calls==5);assert(OSCreateThread(&thread,exitEntry,&work,stack+sizeof(stack),sizeof(stack),7,0));assert(OSResumeThread(&thread)==1);result=nullptr;assert(OSJoinThread(&thread,&result));assert(result==&work&&exitUnwound&&OSIsThreadTerminated(&thread));OSInitMutex(&cancelMutex);OSInitMessageQueue(&cancelQueue,&cancelStorage,1);assert(OSCreateThread(&thread,blockedEntry,nullptr,stack+sizeof(stack),sizeof(stack),7,0));OSResumeThread(&thread);while(!cancelEntered)OSYieldThread();OSDetachThread(&thread);OSCancelThread(&thread);assert(cancelUnwound && OSIsThreadTerminated(&thread));assert(OSCreateThread(&thread,sleepEntry,nullptr,stack+sizeof(stack),sizeof(stack),7,0));assert(OSResumeThread(&thread)==1);while(!sleepEntered)OSYieldThread();std::this_thread::sleep_for(std::chrono::milliseconds(20));const auto cancelStart=std::chrono::steady_clock::now();OSCancelThread(&thread);assert(sleepUnwound&&OSIsThreadTerminated(&thread));assert(std::chrono::steady_clock::now()-cancelStart<std::chrono::seconds(2));assert(OSCreateThread(&thread,entry,&work,stack+sizeof(stack),sizeof(stack),7,0));assert(OSResumeThread(&thread)==1);assert(OSJoinThread(&thread,&result)&&result==&work&&work.calls==6);assert(OSCreateThread(&thread,suspendEntry,&work,stack+sizeof(stack),sizeof(stack),7,0));assert(OSSuspendThread(&thread)==1);assert(OSResumeThread(&thread)==2);assert(OSIsThreadSuspended(&thread));assert(OSResumeThread(&thread)==1);while(!suspensionEntered||!OSIsThreadSuspended(&thread))OSYieldThread();assert(!suspensionReturned);assert(OSResumeThread(&thread)==1);assert(OSJoinThread(&thread,&result)&&result==&work&&suspensionReturned);suspensionEntered=false;suspensionReturned=false;assert(OSCreateThread(&thread,suspendEntry,&work,stack+sizeof(stack),sizeof(stack),7,0));OSResumeThread(&thread);while(!suspensionEntered||!OSIsThreadSuspended(&thread))OSYieldThread();OSCancelThread(&thread);assert(!suspensionReturned&&OSIsThreadTerminated(&thread));std::puts("PASS native OS worker startup, identity, priority metadata, result, synchronized lifecycle queries, join, detached completion and reuse");}










