#include <mutex>
#include <limits>
#include <Native/HeapArena.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
namespace {std::mutex arenaMutex;void* arenaMemory=nullptr;u32 arenaBytes=0;bool consumed=false;}
void* JKRHeap::mCodeStart;void* JKRHeap::mCodeEnd;void* JKRHeap::mUserRamStart;void* JKRHeap::mUserRamEnd;u32 JKRHeap::mMemorySize;
bool nativeBindHeapArena(void* memory,size_t bytes){std::lock_guard<std::mutex> lock(arenaMutex);if(JKRHeap::sRootHeap || arenaMemory || !memory || reinterpret_cast<uintptr_t>(memory)%32 || bytes%32 || bytes<sizeof(JKRExpHeap)+sizeof(JKRExpHeap::CMemBlock)+32 || bytes>std::numeric_limits<u32>::max() || bytes>std::numeric_limits<uintptr_t>::max()-reinterpret_cast<uintptr_t>(memory))return false;arenaMemory=memory;arenaBytes=u32(bytes);consumed=false;return true;}
bool nativeReleaseHeapArena(){std::lock_guard<std::mutex> lock(arenaMutex);if(JKRHeap::sRootHeap)return false;arenaMemory=nullptr;arenaBytes=0;consumed=false;JKRHeap::mUserRamStart=nullptr;JKRHeap::mUserRamEnd=nullptr;JKRHeap::mMemorySize=0;return true;}
bool JKRHeap::initArena(char** memory,u32* size,int maxHeaps){std::lock_guard<std::mutex> lock(arenaMutex);if(!memory || !size || maxHeaps<=0 || !arenaMemory || consumed)return false;*memory=static_cast<char*>(arenaMemory);*size=arenaBytes;consumed=true;mUserRamStart=arenaMemory;mUserRamEnd=static_cast<char*>(arenaMemory)+arenaBytes;mMemorySize=arenaBytes;return true;}
namespace {uintptr_t secondaryLo=0,secondaryHi=0,secondaryEnd=0;}
bool nativeBindSecondaryHeapArena(void* memory,size_t bytes){
    std::lock_guard<std::mutex> lock(arenaMutex);
    const auto address=reinterpret_cast<uintptr_t>(memory);
    if(secondaryLo || !address || address%32 || !bytes || bytes%32 || bytes>std::numeric_limits<u32>::max() || bytes>std::numeric_limits<uintptr_t>::max()-address)return false;
    secondaryLo=address;secondaryHi=secondaryEnd=address+bytes;return true;
}
bool nativeReleaseSecondaryHeapArena(){
    std::lock_guard<std::mutex> lock(arenaMutex);
    // Root hierarchy teardown must precede releasing borrowed storage.
    if(JKRHeap::sRootHeap)return false;
    secondaryLo=secondaryHi=secondaryEnd=0;return true;
}
extern "C" void* OSGetMEM2ArenaLo(){std::lock_guard<std::mutex> lock(arenaMutex);return reinterpret_cast<void*>(secondaryLo);}
extern "C" void* OSGetMEM2ArenaHi(){std::lock_guard<std::mutex> lock(arenaMutex);return reinterpret_cast<void*>(secondaryHi);}
extern "C" void OSSetMEM2ArenaHi(void* memory){
    std::lock_guard<std::mutex> lock(arenaMutex);
    auto address=reinterpret_cast<uintptr_t>(memory);
    if(secondaryLo && address>=secondaryLo && address<=secondaryEnd && address%32==0)secondaryHi=address;
}

