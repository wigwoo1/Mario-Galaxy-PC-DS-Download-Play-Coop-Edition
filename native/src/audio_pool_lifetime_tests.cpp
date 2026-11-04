#include <Native/HeapPlacement.hpp>
#include <Native/AudioPoolHeapLifetime.hpp>
#include <JSystem/JAudio2/JASHeapCtrl.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <windows.h>
#include <cassert>
#include <cstdio>
#include <cstring>

struct alignas(8) LifetimeSlot { void* link; u8 bytes[72]; };
int main() {
    std::setvbuf(stdout,nullptr,_IONBF,0);
    // The actual native pool has static-like lifetime outside both JKR heaps.
    static_assert(sizeof(JASGenericMemPool)==24,"Native cached pool ABI changed");
    JASGenericMemPool pool,second;
    void* memory=VirtualAlloc(nullptr,0x100000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    assert(memory);
    {
        JKRExpHeap root(memory,0x100000,nullptr,false);
        JKRHeap::sRootHeap=JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();
        const auto initial=root.getTotalFreeSize();
        JKRSolidHeap* previous=nullptr;
        for(unsigned cycle=0;cycle<6;++cycle) {
            JASDram=JKRSolidHeap::create(0x40000,&root,false);assert(JASDram);
            auto* heap=JASDram;
            if(previous)assert(heap==previous);previous=heap;
            pool.newMemPool(sizeof(LifetimeSlot),3);
            pool.newMemPool(sizeof(LifetimeSlot),2);
            second.newMemPool(sizeof(LifetimeSlot),2);
            void* slots[5];
            for(auto& slot:slots){slot=pool.alloc(sizeof(LifetimeSlot));assert(slot&&JKRHeap::findFromRoot(slot)==heap);}
            assert(!pool.alloc(sizeof(LifetimeSlot)));
            for(auto* slot:slots)pool.free(slot,sizeof(LifetimeSlot));
            assert(pool.getFreeMemCount()==5&&pool.getUsedMemCount()==5);
            {
                JASGenericMemPool early;
                early.newMemPool(sizeof(LifetimeSlot),2);
                auto* slot=early.alloc(sizeof(LifetimeSlot));assert(slot);
                early.free(slot,sizeof(LifetimeSlot));
            }
            assert(JASDram==heap&&Native::findAudioPoolHeapBinding(&pool));
            // A live pool must not acquire chunks from a different live heap.
            auto* other=JKRSolidHeap::create(0x10000,&root,false);assert(other);
            JASDram=other;bool rejected=false;
            try{pool.newMemPool(sizeof(LifetimeSlot),1);}catch(const std::logic_error&){rejected=true;}
            assert(rejected&&pool.getFreeMemCount()==5&&pool.getTotalMemCount()==5);
            JKRHeap::destroy(other);JASDram=heap;
            JKRHeap::destroy(heap);
            assert(!JASDram&&!Native::findAudioPoolHeapBinding(&pool)&&!Native::findAudioPoolHeapBinding(&second));
            assert(!pool._0&&!pool.getFreeMemCount()&&!pool.getTotalMemCount()&&!pool.getUsedMemCount());
            assert(!second._0&&!second.getTotalMemCount()&&!pool.alloc(sizeof(LifetimeSlot)));
            assert(!Native::audioPoolHeapBindings()&&root.check()&&root.getTotalFreeSize()==initial);
            // Reuse and overwrite the former child-heap/free-list storage.
            auto* churn=root.alloc(0x50000,32);assert(churn);
            std::memset(churn,0xa5,0x50000);root.free(churn);
            assert(!pool.alloc(sizeof(LifetimeSlot))&&root.check());
        }
        std::puts("PASS six same-address heap generations, two longer-lived pools, multi-batch/exhaustion, early pool destruction, cross-heap rejection and overwritten freed storage");
        // Real freeAll uses callAllDisposer, a different disposal entry point.
        JASDram=JKRSolidHeap::create(0x40000,&root,false);auto* heap=JASDram;
        pool.newMemPool(sizeof(LifetimeSlot),3);heap->freeAll();
        assert(!JASDram&&!pool._0&&!pool.getTotalMemCount()&&!Native::audioPoolHeapBindings());
        JASDram=heap;pool.newMemPool(sizeof(LifetimeSlot),3);
        auto* slot=pool.alloc(sizeof(LifetimeSlot));assert(slot);pool.free(slot,sizeof(LifetimeSlot));
        JKRHeap::destroy(heap);assert(!JASDram&&root.check()&&root.getTotalFreeSize()==initial);
        std::puts("PASS original freeAll invalidation, rebinding and containing-heap reclamation");
        // Failure to allocate the observer must leave the pool unmodified.
        JASDram=JKRSolidHeap::create(0x10000,&root,false);heap=JASDram;
        assert(heap->alloc(heap->getFreeSize(),4));bool failed=false;
        try{pool.newMemPool(sizeof(LifetimeSlot),3);}catch(const std::bad_alloc&){failed=true;}
        assert(failed&&!pool._0&&!pool.getTotalMemCount()&&!Native::audioPoolHeapBindings());
        JKRHeap::destroy(heap);JASDram=nullptr;
        assert(root.check()&&root.getTotalFreeSize()==initial);
        std::puts("PASS real observer-allocation failure leaves no binding or changed pool capacity");
    }
    // Force stale free-list access to fail instead of depending on allocator
    // reuse or a readable freed page. No substitute pool/destructor is used.
    assert(VirtualFree(memory,0,MEM_RELEASE));
    assert(!pool.alloc(sizeof(LifetimeSlot)));
    bool rejected=false;try{pool.newMemPool(sizeof(LifetimeSlot),1);}catch(const std::logic_error&){rejected=true;}
    assert(rejected&&!Native::audioPoolHeapBindings());
    std::puts("Allocating root/solid heaps and backing released; destroying the actual longer-lived pool next");
}
