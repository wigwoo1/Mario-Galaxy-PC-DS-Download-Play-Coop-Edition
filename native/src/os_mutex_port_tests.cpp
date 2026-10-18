#include <revolution/types.h>
#include <revolution/os/OSMutex.h>
#ifdef nullptr
#undef nullptr
#endif
#include <cassert>
#include <thread>
#include <vector>
#include <cstdio>
int main(){OSMutex mutex;OSInitMutex(&mutex);auto mainThread=OSGetCurrentThread();assert(mainThread==OSGetCurrentThread());OSLockMutex(&mutex);assert(mutex.count==1 && mutex.thread==mainThread);assert(OSTryLockMutex(&mutex) && mutex.count==2);bool otherAcquired=true;OSThread* workerIdentity=nullptr;std::thread worker([&]{workerIdentity=OSGetCurrentThread();otherAcquired=OSTryLockMutex(&mutex);OSUnlockMutex(&mutex);});worker.join();assert(workerIdentity!=mainThread && !otherAcquired && mutex.count==2);OSUnlockMutex(&mutex);assert(mutex.count==1);OSUnlockMutex(&mutex);assert(mutex.count==0 && mutex.thread==nullptr);
 size_t total=0;std::vector<std::thread> workers;for(int i=0;i<4;++i)workers.emplace_back([&]{for(int job=0;job<20000;++job){OSLockMutex(&mutex);OSLockMutex(&mutex);++total;OSUnlockMutex(&mutex);OSUnlockMutex(&mutex);}});for(auto& thread:workers)thread.join();assert(total==80000 && mutex.count==0 && mutex.thread==nullptr);assert(OSTryLockMutex(&mutex));OSUnlockMutex(&mutex);std::puts("PASS native OS recursive mutex, ownership, try-lock and 80000 threaded updates");}
