#include <initializer_list>
#include "Game/Player/DLchanger.hpp"
#include <cstdint>
#include <cstdio>
static int liveAligned=0,allocationBudget=-1;
void* operator new[](std::size_t size,std::align_val_t alignment){if(allocationBudget==0)throw std::bad_alloc();if(allocationBudget>0)--allocationBudget;void* p=::operator new(size,alignment);++liveAligned;return p;}
void operator delete[](void* p,std::align_val_t alignment) noexcept {if(p){--liveAligned;::operator delete(p,alignment);}}
int main(){
    for(unsigned count : {1u,2u,3u,255u}) {
        DLchanger buffers(static_cast<u8>(count),129);
        for(unsigned i=0;i<count;++i){if(reinterpret_cast<std::uintptr_t>(buffers.mBuffers[i].mDL)%32)return 1;buffers.mBuffers[i].mDL[0]=i;buffers.mBuffers[i].mDL[128]=255-i;}
        for(unsigned step=1;step<=count*2;++step){const auto index=step%count;auto* current=buffers.swap();if(current!=&buffers.mBuffers[index]||current->mDL[0]!=index||current->mDL[128]!=255-index)return 2;}
    }
    if(liveAligned!=0)return 4;allocationBudget=2;bool allocationFailed=false;try{DLchanger partial(4,64);}catch(const std::bad_alloc&){allocationFailed=true;}allocationBudget=-1;if(!allocationFailed||liveAligned!=0)return 5;
    bool rejected=false;try{DLchanger invalid(0,64);}catch(const std::invalid_argument&){rejected=true;}if(!rejected)return 3;
    std::puts("PASS native DLchanger 32-byte buffers, rotation, retained data and destruction");
}

