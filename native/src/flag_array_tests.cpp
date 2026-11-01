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
#include <Native/FlagVertexArrays.hpp>
#include <Native/GxCommandState.hpp>
#include <Native/GDCurrentScope.hpp>
#include <revolution/gx/GXGeometry.h>
#include <vector>
#include <utility>
#include <initializer_list>
#include "original-array-fixture.inc"
int main(){
    std::setvbuf(stdout,nullptr,_IONBF,0);
    alignas(32) static u8 memory[1024*1024]{};
    JKRExpHeap root(memory,sizeof(memory),nullptr,false);
    JKRHeap::sRootHeap=&root;JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();
    u8 commands[128]{};GDLObj writer;GDInitGDLObj(&writer,commands,sizeof(commands));GDSetCurrent(&writer);
    auto* old=new f32[220]{};bool reproduced=false;
    try{GXSetArray(GX_VA_TEX0,old,8);}catch(const std::invalid_argument& e){reproduced=std::strstr(e.what(),"not registered")!=nullptr;}
    delete[] old;assert(reproduced);
    std::puts("PASS reproduce original flag UV failure through real GXSetArray/GDSetArray");
    auto* scene=JKRExpHeap::create(256*1024,&root,false);assert(scene);scene->becomeCurrentHeap();
    const auto freeBefore=scene->getTotalFreeSize();bool invalid=false,overflow=false;
    try{Native::allocateFlagColors(1,10);}catch(const std::invalid_argument&){invalid=true;}
    try{Native::allocateFlagTexCoords(0x7fffffff,0x7fffffff);}catch(const std::bad_array_new_length&){overflow=true;}
    assert(invalid&&overflow&&scene->getTotalFreeSize()==freeBefore);
    std::vector<u32> addresses;std::vector<const void*> pointers;
    unsigned words=0,vertices=0;
    for(auto dimensions:{std::pair<int,int>{10,10},{5,6},{5,5}}){
        OriginalFlagArrayFixture flag{dimensions.first,dimensions.second};flag.init();
        const auto count=Native::flagVertexCount(flag.mNumPointsV,flag.mNumPointsU);
        assert(JKRHeap::findFromRoot(flag.mColors)==scene&&JKRHeap::findFromRoot(flag.mTexST)==scene);
        assert(!(uintptr_t(flag.mColors)&31)&&!(uintptr_t(flag.mTexST)&31));
        GDInitGDLObj(&writer,commands,sizeof(commands));GDSetCurrent(&writer);flag.bind();
        Native::GxCommandState state;state.consume(commands,GDGetCurrOffset(),&Native::gpuResources());
        const auto colorAddress=state.cp(0xa0+GX_VA_CLR0-GX_VA_POS);
        const auto uvAddress=state.cp(0xa0+GX_VA_TEX0-GX_VA_POS);
        assert(colorAddress==Native::gpuResources().byteAddress(flag.mColors));
        assert(uvAddress==Native::gpuResources().byteAddress(flag.mTexST));
        assert(state.cp(0xb0+GX_VA_CLR0-GX_VA_POS)==4&&state.cp(0xb0+GX_VA_TEX0-GX_VA_POS)==8);
        auto* uv=Native::gpuResources().resolve(uvAddress,count*8);
        for(unsigned i=0;i<count*2;++i){u32 bits;std::memcpy(&bits,flag.mTexST+i,4);
            for(unsigned byte=0;byte<4;++byte)assert(uv[i*4+byte]==u8(bits>>(24-byte*8)));++words;}
        auto* color=Native::gpuResources().resolve(colorAddress,count*4);
        for(unsigned i=0;i<count*4;++i)assert(color[i]==255);
        flag.mColors[count-1].set(17,42,93,201);
        GDInitGDLObj(&writer,commands,sizeof(commands));GDSetCurrent(&writer);flag.bind();
        color=Native::gpuResources().resolve(colorAddress,count*4);
        const u8 expected[]={17,42,93,201};assert(std::memcmp(color+(count-1)*4,expected,4)==0);
        flag.mTexST[0]=-0.25f;
        GDInitGDLObj(&writer,commands,sizeof(commands));GDSetCurrent(&writer);flag.bind();
        uv=Native::gpuResources().resolve(uvAddress,count*8);
        assert(uv[0]==0xbe&&uv[1]==0x80&&uv[2]==0&&uv[3]==0);
        bool bounds=false;try{Native::gpuResources().resolve(uvAddress,count*8+1);}catch(const std::out_of_range&){bounds=true;}assert(bounds);
        addresses.push_back(colorAddress);addresses.push_back(uvAddress);pointers.push_back(flag.mColors);pointers.push_back(flag.mTexST);
        vertices+=count;
    }
    root.becomeCurrentHeap();JKRHeap::destroy(scene);
    for(auto pointer:pointers)assert(!Native::hasHeapGpuRange(pointer));
    for(auto address:addresses){bool gone=false;try{Native::gpuResources().resolve(address,1);}catch(const std::out_of_range&){gone=true;}assert(gone);}
    GDSetCurrent(nullptr);
    std::printf("PASS original flag init/bind: 3 grids, %u vertices, %u big-endian float words; raw RGBA updates, strides, bounds and heap teardown\n",vertices,words);
}