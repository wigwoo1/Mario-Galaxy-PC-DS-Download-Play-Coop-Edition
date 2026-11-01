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
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <JSystem/JAudio2/JAIStream.hpp>
#include <cstdlib>
static volatile unsigned constructed=0;
struct StreamSizedProbe: JASPoolAllocObject<StreamSizedProbe>{
    char bytes[sizeof(JAIStream)];
    __attribute__((noinline)) StreamSizedProbe(){++constructed;}
};
int main(){
    std::setvbuf(stdout,nullptr,_IONBF,0);
    alignas(32) static u8 storage[128*1024]{};
    JKRExpHeap root(storage,sizeof(storage),nullptr,false);
    JKRHeap::sRootHeap=&root;JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();
    JASDram=JKRSolidHeap::create(64*1024,&root,false);assert(JASDram);
    StreamSizedProbe::newMemPool(3);
    auto& pool=StreamSizedProbe::memPool_;
    assert(pool.getFreeMemCount()==3&&pool.getTotalMemCount()==3);
    StreamSizedProbe* objects[3]{};
    for(auto& object:objects){object=new StreamSizedProbe;assert(object);}
    assert(constructed==3&&pool.getFreeMemCount()==0);
    const auto before=constructed;
    auto* exhausted=new StreamSizedProbe;
    std::printf("native stream bytes=%zu probe bytes=%zu exhausted=%p constructors before=%u after=%u\n",
        sizeof(JAIStream),sizeof(StreamSizedProbe),static_cast<void*>(exhausted),unsigned(before),unsigned(constructed));
    if(exhausted||constructed!=before){
        std::puts("REPRODUCED: exhausted pool invoked a constructor despite returning null");
        // Only allocation-expression semantics are under test. The existing pool shutdown
        // deallocator is a separate path; do not run static destructors on stack test heaps.
        std::_Exit(2);
    }
    for(auto* object:objects)delete object;
    assert(pool.getFreeMemCount()==3&&pool.getUsedMemCount()==3);
    for(unsigned cycle=0;cycle<100;++cycle){auto* object=new StreamSizedProbe;assert(object);delete object;}
    assert(pool.getFreeMemCount()==3);
    std::puts("PASS pool exhaustion skips construction, three original pool slots reuse across 100 cycles");
    std::_Exit(0);
}