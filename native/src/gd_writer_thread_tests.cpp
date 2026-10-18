#include <Native/CommandSubmission.hpp>
#include <Native/GDCurrentScope.hpp>
#include <Native/AsyncGxCommands.hpp>
#include <revolution/gx.h>
#include <future>
#include <thread>
#include <vector>
#include <cstdio>
#include <cassert>
static GXData testGXData{};
extern "C" GXData* const __GXData=&testGXData;
struct Sink:Native::CommandSubmitter {
 std::vector<u8> bytes;
 void submit(const u8* p,size_t n,bool)override{bytes.assign(p,p+n);}
 void waitForFinish()override{}
 void abortFrame()noexcept override{}
};
int main(){
 u8 mainBytes[128]{};GDLObj mainWriter;GDInitGDLObj(&mainWriter,mainBytes,sizeof(mainBytes));GDSetCurrent(&mainWriter);
 Sink sink;Native::CommandSubmissionBinding binding(sink);GDWriteBPCmd(0x41000011);
 std::promise<void> ready,finished;auto finish=finished.get_future();
 bool workerInitiallyEmpty=false,workerUnchanged=false,workerRestored=false;
 std::thread worker([&]{
  workerInitiallyEmpty=__GDCurrentDL==nullptr;
  {Native::GDCurrentScope restore;u8 privateBytes[128]{};GDLObj privateWriter;
   GDInitGDLObj(&privateWriter,privateBytes,sizeof(privateBytes));GDSetCurrent(&privateWriter);GDWriteBPCmd(0x41000022);
   ready.set_value();finish.wait();workerUnchanged=GDGetCurrOffset()==5&&privateBytes[4]==0x22;
  }
  workerRestored=__GDCurrentDL==nullptr;
 });
 ready.get_future().wait();bool mainUnchanged=__GDCurrentDL==&mainWriter;
 GXFlush();finished.set_value();worker.join();
 bool correctBatch=sink.bytes.size()==37&&sink.bytes[0]==0x61&&sink.bytes[4]==0x11;
 GDSetCurrent(nullptr);
 if(!workerInitiallyEmpty||!workerUnchanged||!workerRestored||!mainUnchanged||!correctBatch){std::puts("FAIL model-thread writer overwrote the render-thread command stream");return 25;}
 std::puts("PASS concurrent private model writer and main GXFlush: isolated initial state, exact command bytes and scoped restoration");
 GDInitGDLObj(&mainWriter,mainBytes,sizeof(mainBytes));GDSetCurrent(&mainWriter);
 std::thread recorded([]{
  for(unsigned n=1;n<=2;++n){Native::AsyncGxCommands commands;GDWriteBPCmd(0x41000000|n);
   {Native::GDCurrentScope restore;u8 local[16]{};GDLObj list;GDInitGDLObj(&list,local,sizeof(local));GDSetCurrent(&list);GDWriteBPCmd(0x410000ee);}
   commands.commit();assert(__GDCurrentDL==nullptr);}
  try{Native::AsyncGxCommands discarded;GDWriteBPCmd(0x410000ff);throw 1;}catch(int){}
 });recorded.join();
 assert(GDGetCurrOffset()==0);Native::drainAsyncGxCommands();
 assert(GDGetCurrOffset()==10&&mainBytes[4]==1&&mainBytes[9]==2);
 GXFlush();assert(sink.bytes.size()==42&&sink.bytes[4]==1&&sink.bytes[9]==2);
 std::thread pending([]{Native::AsyncGxCommands commands;GDWriteBPCmd(0x41000003);commands.commit();});pending.join();
 GDInitGDLObj(&mainWriter,mainBytes,4);bool rejected=false;
 try{Native::drainAsyncGxCommands();}catch(const std::length_error&){rejected=true;}
 assert(rejected&&GDGetCurrOffset()==0);
 GDInitGDLObj(&mainWriter,mainBytes,sizeof(mainBytes));Native::drainAsyncGxCommands();assert(GDGetCurrOffset()==5&&mainBytes[4]==3);
 Native::drainAsyncGxCommands();assert(GDGetCurrOffset()==5);
 {Native::AsyncGxCommands onMain;GDWriteBPCmd(0x41000004);onMain.commit();}
 assert(GDGetCurrOffset()==10&&mainBytes[9]==4);GDSetCurrent(nullptr);
 std::puts("PASS async GX jobs: ordered main-thread replay, nested private lists excluded, failed jobs discarded, atomic capacity retry and main-thread passthrough");
}
