#include <revolution/os/OSThread.h>
#include <revolution/os/OSInterrupt.h>
#include <thread>
#include <atomic>
#include <cassert>
#include <cstdio>
int main(){OSThreadQueue queue;OSInitThreadQueue(&queue);std::atomic<unsigned> entered{0},woke{0};std::thread threads[4];for(auto& thread:threads)thread=std::thread([&]{auto previous=OSDisableInterrupts();++entered;OSSleepThread(&queue);++woke;assert(!OSDisableInterrupts());OSRestoreInterrupts(previous);});while(entered!=4)std::this_thread::yield();OSWakeupThread(&queue);for(auto& thread:threads)thread.join();assert(woke==4);assert(OSDisableInterrupts());assert(!OSDisableInterrupts());OSRestoreInterrupts(true);puts("PASS native thread queue: atomic gate release, wake-all and interrupt state restoration");}
