#include <cassert>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JUtility/JUTException.hpp>
static bool expectingAllocationFailure=false;
static unsigned allocationWarnings=0;
extern "C" void JUTWarningConsole_f(const char* format,...){
    // Only the deliberate exhausted-heap test may emit this original SDK warning.
    assert(expectingAllocationFailure&&std::strcmp(format,":::cannot alloc memory (0x%x byte).\n")==0);
    ++allocationWarnings;
}
extern "C" void JUTReportConsole_f(const char* format,...){
    va_list args;va_start(args,format);std::vfprintf(stderr,format,args);va_end(args);
}
extern "C" void JUTWarningConsole(const char* message){std::fputs(message,stderr);assert(false);}
extern "C" void JUTReportConsole(const char* message){std::fputs(message,stderr);}
void JUTException::panic_f(const char* file,int line,const char* format,...){std::fprintf(stderr,"%s:%d ",file,line);va_list args;va_start(args,format);std::vfprintf(stderr,format,args);va_end(args);assert(false);}
#include <Native/PlayerDisplayList.hpp>
#include <Native/GxCommandState.hpp>
#include <Native/GDCurrentScope.hpp>
#include <Game/Player/DLchanger.hpp>
#include <revolution/gx/GXGeometry.h>
#include <revolution/os/OSCache.h>
#include <vector>
#include <initializer_list>

static bool gone(u32 address){
    try{Native::gpuResources().resolve(address,1);return false;}
    catch(const std::out_of_range&){return true;}
}
static void checkSubmission(u8* bytes,u32 length){
    u8 commands[64]{};GDLObj writer;
    Native::GDCurrentScope restore;
    GDInitGDLObj(&writer,commands,sizeof(commands));GDSetCurrent(&writer);
    GXCallDisplayList(bytes,length);
    assert(GDGetCurrOffset()==9&&commands[0]==0x40);
    Native::GxCommandState state;state.consume(commands,GDGetCurrOffset(),&Native::gpuResources());
    assert(state.xf(0x100a)==0x12345678&&state.bp(0x42)==0x1ab);
}
int main(){std::setvbuf(stdout,nullptr,_IONBF,0);
    alignas(32) static u8 storage[2*1024*1024]{};
    JKRExpHeap root(storage,sizeof(storage),nullptr,false);
    JKRHeap::sRootHeap=&root;JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();
    u8 commands[256]{};GDLObj writer;GDInitGDLObj(&writer,commands,sizeof(commands));GDSetCurrent(&writer);
    auto* old=new(std::align_val_t{32}) u8[256]{};
    bool reproduced=false;try{GXCallDisplayList(old,32);}catch(const std::invalid_argument& e){reproduced=std::strstr(e.what(),"not registered")!=nullptr;}
    ::operator delete[](old,std::align_val_t{32});assert(reproduced&&GDGetCurrOffset()==0);
    std::puts("PASS original GXCallDisplayList reproduces unregistered 256-byte player allocation");

    GDWriteXFCmd(0x100a,0x12345678);GDWriteBPCmd(0x420001ab);GDPadCurr32();
    const u32 length=GDGetCurrOffset();assert(length==32);
    auto* scene=JKRExpHeap::create(512*1024,&root,false);assert(scene);scene->becomeCurrentHeap();
    const auto initialFree=scene->getTotalFreeSize();
    bool zero=false,overflow=false;
    try{Native::allocatePlayerDisplayList(0);}catch(const std::invalid_argument&){zero=true;}
    try{Native::allocatePlayerDisplayList(std::numeric_limits<size_t>::max());}catch(const std::bad_array_new_length&){overflow=true;}
    assert(zero&&overflow&&scene->getTotalFreeSize()==initialFree);

    // Actual capacities of Mario's alternating lighting lists; original update copies/flushes bytes.
    u8* lighting[2]={Native::allocatePlayerDisplayList(0x100),Native::allocatePlayerDisplayList(0x100)};
    u32 addresses[2]{};unsigned current=0;
    for(unsigned frame=0;frame<6;++frame){
        current=1-current;auto* bytes=lighting[current];
        assert(!(uintptr_t(bytes)&31)&&JKRHeap::findFromRoot(bytes)==scene);
        addresses[current]=Native::gpuResources().byteAddress(bytes);
        std::memcpy(bytes,commands,length);DCStoreRange(bytes,length);
        assert(std::memcmp(Native::gpuResources().resolve(addresses[current],length),commands,length)==0);
        checkSubmission(bytes,length);
    }
    auto* bytes=lighting[current];auto address=addresses[current];
    bytes[13]=0xcd;
    assert(Native::gpuResources().resolve(address,32)[13]==0xab); // Snapshot changes only on flush.
    DCFlushRange(bytes,32);
    assert(Native::gpuResources().resolve(address,32)[13]==0xcd);
    bytes[13]=0xab;DCStoreRangeNoSync(bytes,32);checkSubmission(bytes,32);
    bool bounds=false;try{GXCallDisplayList(bytes,257);}catch(const std::out_of_range&){bounds=true;}assert(bounds);
    std::puts("PASS raw GX command bytes, two 256-byte lists, nested replay, stable addresses, flush refresh and bounds");

    // Keep the pre-existing broad rotation test; now require real GPU registration and destruction.
    for(unsigned count:{1u,2u,3u,255u}){
        std::vector<u32> registered;
        {DLchanger buffers(static_cast<u8>(count),129);
            for(unsigned i=0;i<count;++i){auto* p=buffers.mBuffers[i].mDL;assert(!(uintptr_t(p)&31));p[0]=i;p[128]=255-i;registered.push_back(Native::gpuResources().byteAddress(p));}
            for(unsigned step=1;step<=count*2;++step){auto index=step%count;auto* p=buffers.swap();assert(p==&buffers.mBuffers[index]&&p->mDL[0]==index&&p->mDL[128]==255-index);}
        }
        for(auto a:registered)assert(gone(a));
    }
    {DLchanger alpha(2,256);auto* p=alpha.swap();std::memcpy(p->mDL,commands,32);p->mSize=32;DCStoreRange(p->mDL,32);checkSubmission(p->mDL,p->mSize);}
    {auto* texture=Native::allocatePlayerDisplayList(32);std::memcpy(texture,commands,32);DCStoreRange(texture,32);checkSubmission(texture,32);auto a=Native::gpuResources().byteAddress(texture);Native::releasePlayerDisplayList(texture);assert(gone(a));}
    bool countRejected=false;try{DLchanger invalid(0,64);}catch(const std::invalid_argument&){countRejected=true;}assert(countRejected);
    std::puts("PASS DLchanger counts 1/2/3/255, rotations, alpha list replay and explicit unregister-before-free");

    root.becomeCurrentHeap();JKRHeap::destroy(scene);
    for(unsigned i=0;i<2;++i){assert(!Native::hasHeapGpuRange(lighting[i])&&gone(addresses[i]));}
    // Exhaust a real nonfatal scene heap to exercise constructor rollback.
    auto* tiny=JKRExpHeap::create(16*1024,&root,false);assert(tiny);tiny->becomeCurrentHeap();
    const auto ranges=Native::heapGpuRanges().size();const auto freeBefore=tiny->getTotalFreeSize();
    expectingAllocationFailure=true;bool failed=false;try{DLchanger tooLarge(255,256);}catch(const std::bad_alloc&){failed=true;}
    expectingAllocationFailure=false;assert(allocationWarnings==1);assert(failed&&Native::heapGpuRanges().size()==ranges&&tiny->getTotalFreeSize()==freeBefore);
    root.becomeCurrentHeap();JKRHeap::destroy(tiny);
    std::puts("PASS owning scene-heap teardown, real allocation-failure rollback and invalid sizes");
    GDSetCurrent(nullptr);
    return 0;
}