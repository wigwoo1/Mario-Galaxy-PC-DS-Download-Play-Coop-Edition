#include <Native/HeapPlacement.hpp>
#include <JSystem/JAudio2/JASHeapCtrl.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <array>
#include <cassert>
#include <cstdio>
#include <vector>

struct alignas(8) Slot { void* link; u8 bytes[72]; };
int main() {
    std::setvbuf(stdout,nullptr,_IONBF,0);
    std::vector<u8> storage(0x100000+32);
    const auto base=(reinterpret_cast<std::uintptr_t>(storage.data())+31)&~std::uintptr_t(31);
    JKRExpHeap root(reinterpret_cast<void*>(base),0x100000,nullptr,false);
    JKRHeap::sRootHeap=JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();
    const auto initial=root.getTotalFreeSize();
    JASDram=JKRSolidHeap::create(0x40000,&root,false);assert(JASDram);
    for(unsigned cycle=0;cycle<4;++cycle) {
        {
            JASGenericMemPool pool;
            pool.newMemPool(sizeof(Slot),3);pool.newMemPool(sizeof(Slot),2);
            std::array<void*,5> slots{};
            for(auto& slot:slots) {
                slot=pool.alloc(sizeof(Slot));assert(slot&&JKRHeap::findFromRoot(slot)==JASDram);
            }
            assert(!pool.alloc(sizeof(Slot))&&pool.getFreeMemCount()==0&&pool.getTotalMemCount()==5);
            // Nontrivial free-list order: teardown must follow every actual link.
            for(unsigned i:{3u,0u,4u,1u,2u})pool.free(slots[i],sizeof(Slot));
            assert(pool.getFreeMemCount()==5&&pool.getUsedMemCount()==5);
            for(unsigned i=0;i<100;++i){auto* slot=pool.alloc(sizeof(Slot));assert(slot);pool.free(slot,sizeof(Slot));}
            std::printf("destroying real pool cycle=%u first=%p owner=%p JASDram=%p free=%u total=%u\n",cycle,pool._0,JKRHeap::findFromRoot(pool._0),JASDram,pool.getFreeMemCount(),pool.getTotalMemCount());
        }
        assert(root.check()&&JASDram->check());
    }
    std::puts("PASS four real generic-pool destructors, two allocation batches, five slots, exhaustion and100 reuse operations/cycle without replacement global delete");
    JKRHeap::destroy(JASDram);JASDram=nullptr;
    assert(root.check()&&root.getTotalFreeSize()==initial);
    std::puts("PASS containing solid heap reclaimed as one allocation; original parent heap free-size/integrity restored; normal main return");
}
