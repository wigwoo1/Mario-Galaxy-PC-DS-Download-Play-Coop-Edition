#include <Native/HeapPlacement.hpp>
#include <JSystem/JAudio2/JASDvdThread.hpp>
#include <JSystem/JAudio2/JASTaskThread.hpp>
#include <JSystem/JAudio2/JASHeapCtrl.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <windows.h>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <thread>

namespace {
std::mutex mutex;
std::condition_variable changed;
OSThread* mainThread;
unsigned callbacks=0;
void task(void* payload) {
    assert(OSGetCurrentThread()!=mainThread);
    assert(JASDvd::getThreadPointer()&&JASDvd::getThreadPointer()->getThreadRecord()==OSGetCurrentThread());
    unsigned value=0;std::memcpy(&value,payload,sizeof(value));
    std::lock_guard<std::mutex> lock(mutex);
    assert(value==callbacks+1);++callbacks;changed.notify_all();
}
void waitForCommandPoolIdle() {
    auto* heap=JASKernel::getCommandHeap();
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);
    for(;;) {
        OSLockMutex(heap);
        bool empty=true;
        for(auto* chunk=heap->mChunk;chunk;chunk=chunk->getNextChunk())empty&=chunk->isEmpty();
        OSUnlockMutex(heap);
        if(empty)return;
        assert(std::chrono::steady_clock::now()<deadline);
        std::this_thread::yield();
    }
}
}
int main() {
    std::setvbuf(stdout,nullptr,_IONBF,0);mainThread=OSGetCurrentThread();
    for(unsigned generation=0;generation<3;++generation) {
        auto* memory=VirtualAlloc(nullptr,0x400000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);assert(memory);
        {
            JKRExpHeap root(memory,0x400000,nullptr,false);
            JKRHeap::sRootHeap=JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();
            const auto initial=root.getTotalFreeSize();
            auto* dram=JKRSolidHeap::create(0x300000,&root,false);assert(dram);
            JASKernel::setupRootHeap(dram,0x100000);
            for(unsigned cycle=0;cycle<4;++cycle) {
                assert(!JASDvd::getThreadPointer());
                assert(JASDvd::createThread(16,64,0x4000));
                auto* worker=JASDvd::getThreadPointer();assert(worker);
                assert(!JASDvd::createThread(16,64,0x4000)&&JASDvd::getThreadPointer()==worker);
                for(unsigned i=0;i<20;++i) {
                    unsigned value=callbacks+1;
                    if(i&1)assert(worker->sendCmdMsg(task,static_cast<void*>(&value)));
                    else assert(worker->sendCmdMsg(task,&value,sizeof(value)));
                    std::unique_lock<std::mutex> lock(mutex);
                    assert(changed.wait_for(lock,std::chrono::seconds(3),[&]{return callbacks==value;}));
                }
                waitForCommandPoolIdle();
                worker->~JASTaskThread();JKRHeap::free(worker,dram);
                assert(JKRThread::sThreadList.getNumLinks()==0);
                std::printf("DVD generation%u cycle%u destroyed; singleton=%p oldWorker=%p\n",generation+1,cycle+1,JASDvd::getThreadPointer(),worker);
                assert(!JASDvd::getThreadPointer()&&"Destroyed original DVD worker leaves a stale singleton");
            }
            JKRHeap::destroy(dram);JASDram=nullptr;
            assert(root.check()&&root.getTotalFreeSize()==initial);
        }
        assert(VirtualFree(memory,0,MEM_RELEASE));
        std::printf("PASS DVD heap generation%u: actual workers gone, command chunks empty, parent heap fully reclaimed and backing released\n",generation+1);
    }
    assert(callbacks==240&&!JASDvd::getThreadPointer()&&JKRThread::sThreadList.getNumLinks()==0);
    std::puts("PASS 12 actual DVD worker lifetimes/240 ordered callbacks across three released heap generations; normal main return");
}
