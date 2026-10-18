#include <cstring>
#include <thread>
#include <atomic>
#include <new>
#include <cassert>
#include <cstdio>
#include <cstdarg>
#include <JSystem/JKernel/JKRUnitHeap.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JUtility/JUTException.hpp>
#include <Game/Util/Functor.hpp>
#include <Native/HeapFunctor.hpp>
#include <Native/HeapArena.hpp>
#include <Native/InputWorkspace.hpp>
#include <Game/System/HeapMemoryWatcher.hpp>
#include <Game/Util/MemoryUtil.hpp>
#include <Game/Util/MutexHolder.hpp>
#include <Game/System/FunctionAsyncExecutor.hpp>

extern "C" void JUTWarningConsole_f(const char* format,...){va_list args;va_start(args,format);std::vfprintf(stderr,format,args);va_end(args);assert(false);}
extern "C" void JUTReportConsole_f(const char* format,...){va_list args;va_start(args,format);std::vfprintf(stderr,format,args);va_end(args);}
struct Tracked:JKRDisposer{std::atomic<int>* destroyed;explicit Tracked(std::atomic<int>* count):destroyed(count){}~Tracked()override{++*destroyed;}};static_assert(sizeof(Tracked)<=64);
struct JobCallback:MR::FunctorBase{int* calls;int* releases;JobCallback(int* a,int* b):calls(a),releases(b){}~JobCallback(){++*releases;}void operator()()const override{++*calls;}MR::FunctorBase* clone(JKRHeap*)const override{return new JobCallback(calls,releases);}bool dispose()noexcept override{this->~JobCallback();::operator delete(this);return true;}};
void verifyJobPool(){constexpr unsigned unit=(sizeof(FunctionAsyncExecInfo)+31)&~31u;alignas(32) u8 bitmap[32]={};alignas(32) u8 storage[unit*256]={};int calls=0,releases=0;
 JKRUnitHeap heap(bitmap,storage,unit,256,sizeof(storage),32,nullptr,false);JKRHeap::sRootHeap=&heap;
 FunctionAsyncExecInfo* jobs[256];for(auto& job:jobs){void* slot=heap.alloc(sizeof(FunctionAsyncExecInfo),alignof(FunctionAsyncExecInfo));assert(slot);job=new(slot) FunctionAsyncExecInfo(new JobCallback(&calls,&releases),4,"PoolJob");}
 assert(!heap.alloc(sizeof(FunctionAsyncExecInfo),alignof(FunctionAsyncExecInfo)));
 for(auto job:jobs){job->execute();job->~FunctionAsyncExecInfo();heap.free(job);}
 if(!(calls==256 && releases==256 && heap.getTotalFreeSize()==sizeof(storage)))std::fprintf(stderr,"Pool async state calls=%d releases=%d free=%d expectedFree=%zu\n",calls,releases,heap.getTotalFreeSize(),sizeof(storage));
 assert(calls==256 && releases==256 && heap.getTotalFreeSize()==sizeof(storage));
 std::printf("PASS 256 original async jobs: record=%zu slot=%u, callback execution and cleanup\n",sizeof(FunctionAsyncExecInfo),unit);
}
void verifySolidHeap(){alignas(32) u8 storage[4096]={};JKRSolidHeap heap(storage,sizeof(storage),nullptr,false);JKRHeap::sRootHeap=&heap;assert(heap.getMaxAllocatableSize(32)==sizeof(storage));assert(heap.alloc(64,32)==storage);assert(heap.alloc(64,-32)==storage+sizeof(storage)-64);assert(heap.getTotalFreeSize()==sizeof(storage)-128);assert(heap.check());heap.freeTail();assert(heap.getTotalFreeSize()==sizeof(storage)-64);heap.freeAll();assert(heap.getTotalFreeSize()==sizeof(storage));assert(heap.check());}
void verifyFactoryPool(){alignas(32) u8 storage[32768]={};JKRSolidHeap parent(storage,sizeof(storage),nullptr,false);JKRHeap::sRootHeap=&parent;
 constexpr u32 budget=sizeof(JKRUnitHeap)+256*sizeof(FunctionAsyncExecInfo)+32+2*alignof(FunctionAsyncExecInfo);
 auto heap=JKRUnitHeap::create(sizeof(FunctionAsyncExecInfo),budget,alignof(FunctionAsyncExecInfo),&parent,false);assert(heap && heap->getParent()==&parent && heap->mUnitCount>=256);assert(parent.mDisposerList.getNumLinks()==1);parent.becomeCurrentHeap();assert(heap->becomeCurrentHeap()==&parent && JKRHeap::sCurrentHeap==heap);void* selected=JKRHeap::alloc(sizeof(FunctionAsyncExecInfo),alignof(FunctionAsyncExecInfo),nullptr);assert(selected && JKRHeap::findFromRoot(selected)==heap);JKRHeap::free(selected,nullptr);assert(parent.becomeCurrentHeap()==heap);parent.becomeSystemHeap();assert(heap->becomeSystemHeap()==&parent && JKRHeap::sSystemHeap==heap);assert(parent.becomeSystemHeap()==heap);
 OSInitMutex(&MR::MutexHolder<1>::sMutex);std::thread switchers[4];for(auto& switcher:switchers)switcher=std::thread([&]{for(int iteration=0;iteration<1000;++iteration){MR::CurrentHeapRestorer outer(heap);assert(JKRHeap::sCurrentHeap==heap);std::this_thread::yield();{MR::CurrentHeapRestorer inner(&parent);assert(JKRHeap::sCurrentHeap==&parent);}assert(JKRHeap::sCurrentHeap==heap);}});for(auto& switcher:switchers)switcher.join();assert(JKRHeap::sCurrentHeap==&parent);
int calls=0,releases=0;FunctionAsyncExecInfo* jobs[256];for(auto& job:jobs){void* slot=heap->alloc(sizeof(FunctionAsyncExecInfo),alignof(FunctionAsyncExecInfo));assert(slot && reinterpret_cast<uintptr_t>(slot)%alignof(FunctionAsyncExecInfo)==0);job=new(slot) FunctionAsyncExecInfo(new JobCallback(&calls,&releases),4,"FactoryJob");}
 for(auto job:jobs){job->execute();delete job;}assert(calls==256 && releases==256);assert(heap->check());heap->~JKRUnitHeap();assert(parent.mDisposerList.getNumLinks()==0);parent.freeAll();assert(parent.getTotalFreeSize()==sizeof(storage));std::printf("PASS original unit heap factory: budget=%u, 256 native async jobs and parent cleanup\n",budget);
}
extern "C" void JUTWarningConsole(const char* message){std::fputs(message,stderr);assert(false);}
extern "C" void JUTReportConsole(const char* message){std::fputs(message,stderr);}
void JUTException::panic_f(const char* file,int line,const char* format,...){std::fprintf(stderr,"%s:%d ",file,line);va_list args;va_start(args,format);std::vfprintf(stderr,format,args);va_end(args);assert(false);}
void verifyExpHeap(){alignas(32) u8 storage[8192]={};JKRExpHeap heap(storage,sizeof(storage),nullptr,false);JKRHeap::sRootHeap=&heap;const int initial=heap.getTotalFreeSize();void* a=heap.alloc(64,32);void* b=heap.alloc(96,-32);assert(a && b && reinterpret_cast<uintptr_t>(a)%32==0 && reinterpret_cast<uintptr_t>(b)%32==0);assert(heap.check());heap.free(a);heap.free(b);assert(heap.check());assert(heap.getTotalFreeSize()==initial);void* fragmented[32];for(int index=0;index<32;++index){fragmented[index]=heap.alloc(17+index*3,32);assert(fragmented[index]);}
for(int index=0;index<32;index+=2)heap.free(fragmented[index]);assert(heap.check());
for(int index=0;index<32;index+=2){fragmented[index]=heap.alloc(13+index,32);assert(fragmented[index]);}
for(auto block:fragmented)heap.free(block);assert(heap.check() && heap.getTotalFreeSize()==initial);
std::thread workers[4];for(int worker=0;worker<4;++worker)workers[worker]=std::thread([&,worker]{for(int iteration=0;iteration<5000;++iteration){const unsigned size=17+(iteration%13)*3;void* block=heap.alloc(size,worker%2?-32:32);assert(block && reinterpret_cast<uintptr_t>(block)%32==0);auto bytes=static_cast<u8*>(block);for(unsigned offset=0;offset<size;++offset)bytes[offset]=u8(worker+1);std::this_thread::yield();for(unsigned offset=0;offset<size;++offset)assert(bytes[offset]==worker+1);heap.free(block);}});
for(auto& worker:workers)worker.join();assert(heap.check() && heap.getTotalFreeSize()==initial);
{
 alignas(32) u8 secondStorage[8192]={};JKRExpHeap second(secondStorage,sizeof(secondStorage),&heap,false);const int secondInitial=second.getTotalFreeSize();
 JKRExpHeap* heaps[2]={&heap,&second};std::thread independent[2];
 for(int worker=0;worker<2;++worker)independent[worker]=std::thread([&,worker]{auto allocator=heaps[worker];for(int iteration=0;iteration<10000;++iteration){const unsigned size=29+(iteration%17)*5;void* block=allocator->alloc(size,32);assert(block);auto bytes=static_cast<u8*>(block);for(unsigned offset=0;offset<size;++offset)bytes[offset]=u8(worker+3);std::this_thread::yield();for(unsigned offset=0;offset<size;++offset)assert(bytes[offset]==worker+3);allocator->free(block);}});
 for(auto& worker:independent)worker.join();assert(heap.check() && second.check());assert(heap.getTotalFreeSize()==initial && second.getTotalFreeSize()==secondInitial);
}
std::puts("PASS original expandable heap fragmentation, shared-heap and separate-heap concurrent allocation cycles");}
struct FunctorReceiver{int value=0;void increment(){++value;}void add(int amount,int* output){value+=amount;*output=value;}};
void verifyExpFactory(){alignas(32) u8 storage[16384]={};JKRSolidHeap parent(storage,sizeof(storage),nullptr,false);JKRHeap::sRootHeap=&parent;
 auto heap=JKRExpHeap::create(0x2800,&parent,false);assert(heap && heap->getParent()==&parent && parent.mDisposerList.getNumLinks()==1);const int initial=heap->getTotalFreeSize();void* blocks[64];for(unsigned i=0;i<64;++i){blocks[i]=heap->alloc(24+i,16);assert(blocks[i] && reinterpret_cast<uintptr_t>(blocks[i])%16==0);assert(JKRHeap::findFromRoot(blocks[i])==heap);}assert(heap->check());for(auto block:blocks)heap->free(block);
int calls=0,releases=0;{JobCallback callback(&calls,&releases);auto owned=MR::nativeCloneHeapCallback(callback,heap);assert(JKRHeap::findFromRoot(owned)==heap);auto copied=owned->clone(heap);(*owned)();(*copied)();assert(owned->dispose());assert(copied->dispose());assert(calls==2 && releases==2);}assert(releases==3);{JobCallback callback(&calls,&releases);{FunctionAsyncExecInfo job(MR::nativeCloneHeapCallback(callback,heap),4,"HeapOwnedJob");job.execute();}assert(calls==3 && releases==4);}assert(releases==5);FunctorReceiver receiver;int output=0;auto zero=MR::Functor(&receiver,&FunctorReceiver::increment);auto two=MR::Functor(&receiver,&FunctorReceiver::add,5,&output);{FunctionAsyncExecInfo job(zero.clone(heap),4,"HeapFunctorZero");job.execute();}{FunctionAsyncExecInfo job(two.clone(heap),4,"HeapFunctorTwo");job.execute();}assert(receiver.value==6 && output==6);assert(heap->check() && heap->getTotalFreeSize()==initial);heap->~JKRExpHeap();assert(parent.mDisposerList.getNumLinks()==0);parent.freeAll();assert(parent.getTotalFreeSize()==sizeof(storage));std::puts("PASS original expandable heap factory: callback budget, ownership lookup, allocation/free and parent cleanup");
}
void* dispatchHeapJob(void* raw){static_cast<FunctionAsyncExecutorOnMainThread*>(raw)->update();return nullptr;}
void verifyWorkerHeapJobs(){alignas(32) u8 storage[49152]={};JKRSolidHeap parent(storage,sizeof(storage),nullptr,false);JKRHeap::sRootHeap=&parent;
 auto records=JKRUnitHeap::create(sizeof(FunctionAsyncExecInfo),sizeof(JKRUnitHeap)+256*sizeof(FunctionAsyncExecInfo)+32+2*alignof(FunctionAsyncExecInfo),alignof(FunctionAsyncExecInfo),&parent,false);auto callbacks=JKRExpHeap::create(0x2800,&parent,false);assert(records && callbacks);const int recordFree=records->getTotalFreeSize(),callbackFree=callbacks->getTotalFreeSize();FunctorReceiver receiver;auto function=MR::Functor(&receiver,&FunctorReceiver::increment);FunctionAsyncExecutorOnMainThread executor(OSGetCurrentThread());
 for(int iteration=0;iteration<32;++iteration){auto job=new(records,alignof(FunctionAsyncExecInfo)) FunctionAsyncExecInfo(function.clone(callbacks),4,"WorkerHeapJob");assert(OSSendMessage(&executor.mQueue,job,OS_MESSAGE_NOBLOCK));OSThread worker;char stack[4096];assert(OSCreateThread(&worker,dispatchHeapJob,&executor,stack+sizeof(stack),sizeof(stack),7,0));assert(OSResumeThread(&worker)==1);OSMessage result=reinterpret_cast<void*>(1);assert(OSReceiveMessage(&job->mQueue,&result,OS_MESSAGE_BLOCK));assert(result==nullptr && job->mIsEnd && receiver.value==iteration+1);delete job;assert(OSJoinThread(&worker,nullptr));assert(records->getTotalFreeSize()==recordFree && callbacks->getTotalFreeSize()==callbackFree);}
 assert(records->check() && callbacks->check());callbacks->~JKRExpHeap();records->~JKRUnitHeap();assert(parent.mDisposerList.getNumLinks()==0);parent.freeAll();std::puts("PASS original worker dispatch, completion and immediate deletion of heap-owned job records/callbacks");
}
void verifyRootArena(){alignas(32) u8 storage[65536]={};assert(!JKRExpHeap::createRoot(1,false));assert(!nativeBindHeapArena(storage+1,sizeof(storage)-32));assert(nativeBindHeapArena(storage,sizeof(storage)));assert(!nativeBindHeapArena(storage,sizeof(storage)));auto root=JKRExpHeap::createRoot(8,false);assert(root && JKRHeap::sRootHeap==root && root->check());assert(!nativeReleaseHeapArena());auto block=root->alloc(128,32);assert(block && JKRHeap::findFromRoot(block)==root);root->free(block);assert(root->check());root->~JKRExpHeap();assert(!JKRHeap::sRootHeap);assert(nativeReleaseHeapArena());std::puts("PASS native arena binding and original root heap startup/allocation/teardown");}
extern "C" void* OSGetMEM2ArenaLo();
extern "C" void* OSGetMEM2ArenaHi();
extern "C" void OSSetMEM2ArenaHi(void*);
void verifySecondaryArena(){
    alignas(32) u8 storage[4096]={};
    assert(!OSGetMEM2ArenaLo() && !OSGetMEM2ArenaHi());
    assert(!nativeBindSecondaryHeapArena(storage+1,4096));
    assert(!nativeBindSecondaryHeapArena(storage,4095));
    assert(nativeBindSecondaryHeapArena(storage,sizeof(storage)));
    JKRHeap::setAltAramStartAdr(reinterpret_cast<uintptr_t>(storage));assert(JKRHeap::getAltAramStartAdr()==reinterpret_cast<uintptr_t>(storage));JKRHeap::setAltAramStartAdr(0);
    assert(!nativeBindSecondaryHeapArena(storage,sizeof(storage)));
    assert(OSGetMEM2ArenaLo()==storage && OSGetMEM2ArenaHi()==storage+4096);
    OSSetMEM2ArenaHi(storage+2048);assert(OSGetMEM2ArenaHi()==storage+2048);
    OSSetMEM2ArenaHi(storage+1);assert(OSGetMEM2ArenaHi()==storage+2048);
    OSSetMEM2ArenaHi(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(storage)-32));assert(OSGetMEM2ArenaHi()==storage+2048);
    OSSetMEM2ArenaHi(storage+4096);assert(OSGetMEM2ArenaHi()==storage+4096);
    OSSetMEM2ArenaHi(storage);assert(OSGetMEM2ArenaHi()==storage);
    assert(nativeReleaseSecondaryHeapArena());assert(!OSGetMEM2ArenaLo() && !OSGetMEM2ArenaHi());
    std::puts("PASS bounded secondary arena binding, shrink/restore and invalid bounds rejection");
}
void verifyOriginalRootHierarchy(){
    alignas(32) u8 primary[65536]={};
    const size_t secondaryBytes=32*1024*1024;
    auto secondary=static_cast<u8*>(::operator new(secondaryBytes,std::align_val_t(32)));
    assert(nativeBindHeapArena(primary,sizeof(primary)));
    assert(nativeBindSecondaryHeapArena(secondary,secondaryBytes));
    HeapMemoryWatcher::createRootHeap();
    auto root=JKRHeap::sRootHeap;auto second=HeapMemoryWatcher::sRootHeapGDDR3;
    assert(root && second && root!=second && root->check() && second->check());
    assert(JKRHeap::getAltAramStartAdr()==reinterpret_cast<uintptr_t>(secondary));
    assert(OSGetMEM2ArenaHi()==secondary+0xE00000);
    assert(JKRHeap::sCurrentHeap==root);
    auto block=second->alloc(4096,32);assert(block && JKRHeap::findFromRoot(block)==second);second->free(block);
    assert(second->check());JKRHeap::destroy(second);HeapMemoryWatcher::sRootHeapGDDR3=nullptr;
    JKRHeap::destroy(root);JKRHeap::setAltAramStartAdr(0);
    assert(nativeReleaseSecondaryHeapArena() && nativeReleaseHeapArena());
    ::operator delete(secondary,std::align_val_t(32));
    std::puts("PASS original HeapMemoryWatcher two-arena root startup, allocation and teardown");
}
void verifyExternalHeapValidation(){
    alignas(32) u8 storage[4096]={};
    assert(!JKRExpHeap::create(u32(0xffffffff),nullptr,false));
    assert(!JKRExpHeap::create(nullptr,sizeof(storage),nullptr,false));
    assert(!JKRExpHeap::create(storage,sizeof(storage),nullptr,false));
    auto parent=reinterpret_cast<JKRHeap*>(uintptr_t(1));
    assert(!JKRExpHeap::create(nullptr,sizeof(storage),parent,false));
    assert(!JKRExpHeap::create(storage+1,sizeof(storage)-1,parent,false));
    assert(!JKRExpHeap::create(storage,0,parent,false));
    assert(!JKRExpHeap::create(storage,sizeof(JKRExpHeap),parent,false));
    std::puts("PASS external expandable heap rejects null, unaligned, undersized and parentless storage");
}
void verifyFullHeapHierarchy(){
    constexpr size_t primaryBytes=24*1024*1024,secondaryBytes=64*1024*1024;
    auto primary=::operator new(primaryBytes,std::align_val_t(32));
    auto secondary=::operator new(secondaryBytes,std::align_val_t(32));
    assert(!nativeConfigureInputWorkspace(0));assert(!nativeConfigureInputWorkspace(0xffffffff));
    assert(nativeConfigureInputWorkspace(65536));
    assert(nativeBindHeapArena(primary,primaryBytes) && nativeBindSecondaryHeapArena(secondary,secondaryBytes));
    HeapMemoryWatcher::createRootHeap();assert(!nativeConfigureInputWorkspace(4096));
    HeapMemoryWatcher watcher;
    JKRHeap* heaps[]={JKRHeap::sSystemHeap,watcher.mAudSystemHeap,watcher.mStationedHeapNapa,watcher.mWPadHeap,watcher.mHomeButtonLayoutHeap,watcher.mStationedHeapGDDR,watcher.mGameHeapNapa,watcher.mGameHeapGDDR};
    for(auto heap:heaps){assert(heap && heap->check());auto block=heap->alloc(256,16);assert(block && JKRHeap::findFromRoot(block)==heap);if(heap!=watcher.mAudSystemHeap)heap->free(block);}
    assert(JKRHeap::sCurrentHeap==JKRHeap::sRootHeap);
    assert(watcher.mGameHeapNapa->resize(reinterpret_cast<void*>(uintptr_t(1)),64)==-1);assert(watcher.mGameHeapNapa->do_getSize(reinterpret_cast<void*>(uintptr_t(1)))==-1);
    auto stationed=watcher.mStationedHeapNapa;
    auto stationedBlock=stationed->alloc(1024,32);assert(stationedBlock);
    std::memset(stationedBlock,0x5a,1024);
    const auto parentFreeBefore=JKRHeap::sRootHeap->getTotalFreeSize();
    assert(stationed->adjustSize()>0 && stationed->check() && JKRHeap::sRootHeap->check());
    assert(JKRHeap::sRootHeap->getTotalFreeSize()>parentFreeBefore);
    for(int i=0;i<1024;++i)assert(static_cast<u8*>(stationedBlock)[i]==0x5a);
    assert(JKRHeap::findFromRoot(stationedBlock)==stationed);
    stationed->free(stationedBlock);assert(stationed->check());
    const auto secondaryFreeBefore=HeapMemoryWatcher::sRootHeapGDDR3->getTotalFreeSize();
    watcher.adjustStationedHeaps();assert(watcher.mStationedHeapNapa->check() && watcher.mStationedHeapGDDR->check());
    assert(HeapMemoryWatcher::sRootHeapGDDR3->getTotalFreeSize()>secondaryFreeBefore);
    assert(static_cast<JKRExpHeap*>(JKRHeap::sRootHeap)->adjustSize()==-1);
    watcher.destroyGameHeap();
    const auto napaFree=watcher.mGameHeapNapa->getFreeSize();
    const auto gddrFree=watcher.mGameHeapGDDR->getFreeSize();
    std::atomic<int> sceneDestroyed{0};
    for(int cycle=0;cycle<8;++cycle){
        watcher.createFileCacheHeapOnGameHeap(65536);
        watcher.createSceneHeapOnGameHeap();
        assert(watcher.mFileCacheHeap && watcher.mSceneHeapNapa && watcher.mSceneHeapGDDR);
        watcher.setCurrentHeapToSceneHeap();assert(JKRHeap::sCurrentHeap==watcher.mSceneHeapNapa);
        auto sceneBlock=JKRHeap::alloc(4096,32,nullptr);assert(sceneBlock && JKRHeap::findFromRoot(sceneBlock)==watcher.mSceneHeapNapa);
        auto tracked=new(watcher.mSceneHeapNapa->alloc(sizeof(Tracked),alignof(Tracked))) Tracked(&sceneDestroyed);assert(tracked->mHeap==watcher.mSceneHeapNapa);
        assert(watcher.mSceneHeapNapa->check() && watcher.mSceneHeapGDDR->check());
        watcher.setCurrentHeapToGameHeap();assert(JKRHeap::sCurrentHeap==JKRHeap::sSystemHeap);
        watcher.destroySceneHeap();assert(!watcher.mSceneHeapNapa && !watcher.mSceneHeapGDDR);assert(sceneDestroyed==cycle+1);
        watcher.destroyGameHeap();assert(!watcher.mFileCacheHeap && watcher.mGameHeapNapa && watcher.mGameHeapGDDR);
        assert(watcher.mGameHeapNapa->getFreeSize()==napaFree && watcher.mGameHeapGDDR->getFreeSize()==gddrFree);
        assert(watcher.mGameHeapNapa->check() && watcher.mGameHeapGDDR->check());
    }
    JKRHeap::sRootHeap->becomeCurrentHeap();
    std::puts("PASS original scene/cache creation, heap selection and eight game-heap reset cycles");
    JKRHeap::destroy(watcher.mGameHeapGDDR);JKRHeap::destroy(watcher.mGameHeapNapa);
    for(int i=5;i>=0;--i)JKRHeap::destroy(heaps[i]);
    JKRHeap::destroy(HeapMemoryWatcher::sRootHeapGDDR3);HeapMemoryWatcher::sRootHeapGDDR3=nullptr;
    JKRHeap::destroy(JKRHeap::sRootHeap);JKRHeap::setAltAramStartAdr(0);
    assert(nativeReleaseSecondaryHeapArena() && nativeReleaseHeapArena());
    ::operator delete(secondary,std::align_val_t(32));::operator delete(primary,std::align_val_t(32));
    std::puts("PASS original full HeapMemoryWatcher constructor and system/audio/input/stationed/game heaps");
}
void verifyLayoutHeapAllocator(){alignas(32) u8 storage[65536]={};JKRSolidHeap parent(storage,sizeof(storage),nullptr,false);JKRHeap::sRootHeap=&parent;auto heap=JKRExpHeap::create(49152,&parent,false);assert(heap);auto capacity=heap->getTotalFreeSize();MR::JKRHeapAllocator<77>::sHeap=heap;
void* blocks[128];for(unsigned i=0;i<128;++i){blocks[i]=MEMAllocFromAllocator(&MR::JKRHeapAllocator<77>::sAllocator,i+1);assert(blocks[i] && reinterpret_cast<uintptr_t>(blocks[i])%alignof(std::max_align_t)==0);assert(JKRHeap::findFromRoot(blocks[i])==heap);std::memset(blocks[i],int(i),i+1);}
for(unsigned i=0;i<128;++i){auto bytes=static_cast<u8*>(blocks[i]);for(unsigned j=0;j<=i;++j)assert(bytes[j]==u8(i));MEMFreeToAllocator(&MR::JKRHeapAllocator<77>::sAllocator,blocks[i]);}
assert(heap->check() && heap->getTotalFreeSize()==capacity);MR::JKRHeapAllocator<77>::sHeap=nullptr;heap->~JKRExpHeap();parent.freeAll();std::puts("PASS original layout heap callbacks: 128 varied allocations, host alignment, payload and full recovery");}
void verifyLayoutDefaultAllocator(){assert(!JKRHeap::sCurrentHeap);assert(!MEMAllocFromAllocator(&MR::NewDeleteAllocator::sAllocator,64));alignas(32) u8 storage[65536]={};JKRSolidHeap parent(storage,sizeof(storage),nullptr,false);JKRHeap::sRootHeap=&parent;auto first=JKRExpHeap::create(24576,&parent,false),second=JKRExpHeap::create(24576,&parent,false);assert(first && second);auto a=first->getTotalFreeSize(),b=second->getTotalFreeSize();first->becomeCurrentHeap();auto block=static_cast<u8*>(MEMAllocFromAllocator(&MR::NewDeleteAllocator::sAllocator,257));assert(block && JKRHeap::findFromRoot(block)==first && reinterpret_cast<uintptr_t>(block)%alignof(std::max_align_t)==0);std::memset(block,0x5a,257);second->becomeCurrentHeap();for(unsigned i=0;i<257;++i)assert(block[i]==0x5a);MEMFreeToAllocator(&MR::NewDeleteAllocator::sAllocator,block);assert(first->getTotalFreeSize()==a && second->getTotalFreeSize()==b);MEMFreeToAllocator(&MR::NewDeleteAllocator::sAllocator,nullptr);second->~JKRExpHeap();first->~JKRExpHeap();parent.freeAll();std::puts("PASS default layout allocator: current game heap allocation and owner-based free after heap switch");}
int main(){std::setvbuf(stdout,nullptr,_IONBF,0);alignas(32) u8 bitmap[4]={};alignas(32) u8 slots[32*64]={};std::atomic<int> destroyed{0};{
 JKRUnitHeap heap(bitmap,slots,64,32,sizeof(slots),32,nullptr,false);JKRHeap::sRootHeap=&heap;
 void* allocations[32];for(int i=0;i<32;++i){allocations[i]=heap.alloc(64,32);assert(allocations[i]==slots+i*64);assert(heap.do_getSize(allocations[i])==64);}
 assert(heap.getTotalFreeSize()==0);assert(heap.getFreeSize()==0 && !heap.getMaxFreeBlock());assert(!heap.alloc(1,32));heap.free(allocations[7]);assert(heap.getTotalFreeSize()==64);assert(heap.getFreeSize()==64 && heap.getMaxFreeBlock()==allocations[7]);heap.free(allocations[7]);assert(heap.getTotalFreeSize()==64);assert(heap.getFreeSize()==64 && heap.getMaxFreeBlock()==allocations[7]);assert(heap.alloc(64,32)==allocations[7]);heap.free(slots+1);assert(heap.getTotalFreeSize()==0);heap.freeAll();assert(heap.getTotalFreeSize()==sizeof(slots));assert(heap.getFreeSize()==sizeof(slots) && heap.getMaxFreeBlock()==slots);assert(heap.alloc(64,-32)==slots+31*64);assert(!heap.alloc(65,32));assert(!heap.alloc(64,64));assert(heap.check());heap.freeAll();void* storage=heap.alloc(64,32);auto object=new(storage) Tracked(&destroyed);assert(object->mHeap==&heap);assert(heap.mDisposerList.getNumLinks()==1);heap.free(storage);assert(destroyed==1 && heap.mDisposerList.getNumLinks()==0);heap.free(storage);assert(destroyed==1);storage=heap.alloc(64,32);object=new(storage) Tracked(&destroyed);heap.dispose();assert(destroyed==2 && heap.mDisposerList.getNumLinks()==0);heap.free(storage);assert(destroyed==2);std::atomic<int> owners[32]{};std::thread workers[4];
for(int worker=0;worker<4;++worker)workers[worker]=std::thread([&,worker]{
 for(int iteration=0;iteration<10000;++iteration){const int reported=heap.getTotalFreeSize();assert(reported>=0 && reported<=sizeof(slots) && reported%64==0);
  void* block=nullptr;while(!(block=heap.alloc(64,32)))std::this_thread::yield();
  const int index=heap.addressToIndex(block);assert(index>=0 && index<32);assert(owners[index].exchange(worker+1)==0);
  auto bytes=static_cast<u8*>(block);for(int offset=0;offset<64;++offset)bytes[offset]=u8(worker+1);
  std::this_thread::yield();for(int offset=0;offset<64;++offset)assert(bytes[offset]==worker+1);
  assert(owners[index].exchange(0)==worker+1);heap.free(block);
 }
});
for(auto& worker:workers)worker.join();assert(heap.getTotalFreeSize()==sizeof(slots));assert(heap.check());
for(int worker=0;worker<4;++worker)workers[worker]=std::thread([&]{for(int iteration=0;iteration<5000;++iteration){void* block=nullptr;while(!(block=heap.alloc(64,32)))std::this_thread::yield();new(block) Tracked(&destroyed);std::this_thread::yield();heap.free(block);}});
for(auto& worker:workers)worker.join();assert(destroyed==20002);assert(heap.mDisposerList.getNumLinks()==0);assert(heap.getTotalFreeSize()==sizeof(slots));
storage=heap.alloc(64,32);new(storage) Tracked(&destroyed);
 }assert(destroyed==20003);assert(!JKRHeap::sRootHeap && !JKRHeap::sCurrentHeap && !JKRHeap::sSystemHeap);verifyJobPool();verifySolidHeap();verifyFactoryPool();verifyExpHeap();verifyExpFactory();verifyWorkerHeapJobs();verifyRootArena();verifySecondaryArena();verifyOriginalRootHierarchy();verifyExternalHeapValidation();verifyFullHeapHierarchy();verifyLayoutHeapAllocator();verifyLayoutDefaultAllocator();std::puts("PASS constructed original unit heap allocation, exhaustion, reuse, validation, tail allocation, registered disposal, 40000 concurrent allocation cycles and teardown");}





































