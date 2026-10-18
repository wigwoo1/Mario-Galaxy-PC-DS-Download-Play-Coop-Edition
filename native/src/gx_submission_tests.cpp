#include <Native/CommandSubmission.hpp>
#include <revolution/gx.h>
#include <revolution/gd/GDBase.h>
#include <vector>
#include <cassert>
#include <cstdio>
extern "C" void GXWaitDrawDone();
static GXData testGXData{};
extern "C" GXData* const __GXData=&testGXData;
struct Fixture:Native::CommandSubmitter {
 std::vector<u8> bytes;bool finish=false,waited=false,reject=false,aborted=false;
 void submit(const u8* data,std::size_t size,bool marker)override{if(reject)throw std::runtime_error("Rejected fixture batch");bytes.assign(data,data+size);finish=marker;}
 void waitForFinish()override{assert(finish);waited=true;}
 void abortFrame()noexcept override{aborted=true;}
};
int main(){
 u8 bytes[128]{};GDLObj dl;GDInitGDLObj(&dl,bytes,sizeof(bytes));GDSetCurrent(&dl);Fixture fixture;
 bool rejected=false;try{GXFlush();}catch(const std::logic_error&){rejected=true;}assert(rejected&&GDGetCurrOffset()==0);
 {Native::CommandSubmissionBinding binding(fixture);
 GDWriteBPCmd(0x41000008);GXFlush();assert(fixture.bytes.size()==37&&!fixture.finish&&GDGetCurrOffset()==0);for(unsigned i=5;i<37;++i)assert(fixture.bytes[i]==0);
 GXDrawDone();assert(fixture.finish&&fixture.waited&&fixture.bytes.size()==37&&fixture.bytes[0]==0x61&&fixture.bytes[1]==0x45&&fixture.bytes[4]==2);
 GDWriteBPCmd(0x41000010);fixture.reject=true;
 rejected=false;try{GXSetDrawDone();}catch(const std::runtime_error&){rejected=true;}
 assert(rejected&&GDGetCurrOffset()==5&&bytes[0]==0x61&&bytes[1]==0x41&&bytes[4]==0x10);
 fixture.reject=false;GXSetDrawDone();assert(GDGetCurrOffset()==0&&fixture.bytes.size()==42&&fixture.bytes[5]==0x61&&fixture.bytes[6]==0x45&&fixture.bytes[9]==2);
  GXSetDrawSync(0xabcd);assert(fixture.bytes.size()==42&&!fixture.finish&&GDGetCurrOffset()==0);
 assert(fixture.bytes[0]==0x61&&fixture.bytes[1]==0x48&&fixture.bytes[2]==0&&fixture.bytes[3]==0xab&&fixture.bytes[4]==0xcd);
 assert(fixture.bytes[5]==0x61&&fixture.bytes[6]==0x47&&fixture.bytes[7]==0&&fixture.bytes[8]==0xab&&fixture.bytes[9]==0xcd);
 for(size_t i=10;i<42;++i)assert(fixture.bytes[i]==0);
 GDWriteBPCmd(0x41000010);fixture.reject=true;rejected=false;
 try{GXSetDrawSync(7);}catch(const std::runtime_error&){rejected=true;}
 assert(rejected&&GDGetCurrOffset()==5&&bytes[1]==0x41&&bytes[4]==0x10);
 fixture.reject=false;GXSetDrawSync(0);assert(fixture.bytes.size()==47&&fixture.bytes[6]==0x48&&fixture.bytes[11]==0x47);
 GDWriteBPCmd(0x41000020);GXAbortFrame();assert(fixture.aborted&&GDGetCurrOffset()==0&&testGXData.dirtyState==0);
 Native::appendGxAbortResetCommand();assert(GDGetCurrOffset()==5&&bytes[0]==0x61&&bytes[1]==0x58&&bytes[4]==0x0f);
 fixture.waited=false;GXSetDrawDone();Native::markGxAbortRecoveryReady();GXWaitDrawDone();assert(fixture.waited);
 GDInitGDLObj(&dl,bytes,41);rejected=false;try{GXSetDrawSync(0xffff);}catch(const std::length_error&){rejected=true;}
 assert(rejected&&GDGetCurrOffset()==0);
GDInitGDLObj(&dl,bytes,36);rejected=false;try{GXSetDrawDone();}catch(const std::length_error&){rejected=true;}assert(rejected&&GDGetCurrOffset()==0);
 }GDSetCurrent(nullptr);std::puts("PASS GX submission bridge: full FIFO tail, finish marker, wait ordering, unbound/capacity rejection");
}


