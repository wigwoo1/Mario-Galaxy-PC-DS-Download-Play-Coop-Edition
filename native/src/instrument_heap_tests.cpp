#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JAudio2/JASBasicInst.hpp>
#include <cassert>
#include <JSystem/JUtility/JUTException.hpp>
#include <cstdio>
#include <cstdarg>
extern "C" void JUTWarningConsole_f(const char*,...){assert(false);}
extern "C" void JUTReportConsole_f(const char*,...){}
extern "C" void JUTWarningConsole(const char* message){std::fputs(message,stderr);assert(false);}
extern "C" void JUTReportConsole(const char* message){std::fputs(message,stderr);}
void JUTException::panic_f(const char* file,int line,const char* format,...){std::fprintf(stderr,"%s:%d ",file,line);va_list args;va_start(args,format);std::vfprintf(stderr,format,args);va_end(args);assert(false);}
int main(){alignas(32) u8 arena[131072]{};JKRSolidHeap parent(arena,sizeof(arena),nullptr,false);JKRHeap::sRootHeap=&parent;auto* owner=JKRExpHeap::create(49152,&parent,false);auto* other=JKRExpHeap::create(49152,&parent,false);assert(owner&&other);owner->becomeCurrentHeap();auto before=owner->getTotalFreeSize();auto otherBefore=other->getTotalFreeSize();{
JASBasicInst inst;inst.setKeyRegionCount(3,owner);for(int j=0;j<3;++j){auto* key=inst.getKeyRegion(j);assert(key&&key->mVelomap==nullptr);key->setVeloRegionCount(7,owner);assert(JKRHeap::findFromRoot(key->mVelomap)==owner);key->getVeloRegion(6)->_4=123;}
other->becomeCurrentHeap();inst.getKeyRegion(1)->setVeloRegionCount(2,owner);assert(owner->check()&&other->check());inst.setKeyRegionCount(1,owner);inst.getKeyRegion(0)->setVeloRegionCount(9,owner);inst.setKeyRegionCount(0,owner);assert(owner->getTotalFreeSize()==before);inst.setKeyRegionCount(2,owner);inst.getKeyRegion(0)->setVeloRegionCount(3,owner);
}assert(owner->check()&&other->check());assert(owner->getTotalFreeSize()==before&&other->getTotalFreeSize()==otherBefore);parent.becomeCurrentHeap();other->~JKRExpHeap();owner->~JKRExpHeap();parent.freeAll();puts("PASS original instrument regions: heap owner replacement, zero count, nested teardown and full free-space restoration after heap switch");}

