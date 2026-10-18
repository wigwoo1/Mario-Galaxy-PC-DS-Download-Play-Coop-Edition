#include <Native/CommandSubmission.hpp>
#include <Native/AsyncGxCommands.hpp>
#include <Native/GxCommandObservation.hpp>
#include <revolution/gx.h>
#include <revolution/gd/GDBase.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <vector>
namespace {Native::CommandSubmitter* backend=nullptr;
std::atomic<bool> completionPending{false};
std::mutex writerMutex;
std::mutex asyncCommandsMutex;
std::vector<std::vector<u8>> asyncCommands;
std::mutex abortRecoveryMutex;
std::condition_variable abortRecoveryChanged;
bool abortRecoveryPending=false;
bool abortRecoveryReady=false;
void flush(bool finish){
 if(!backend)throw std::logic_error("No native GX command submitter");
 std::vector<u8> batch;
 GDLObj* writer=nullptr;
 u8* originalEnd=nullptr;
 // Original GX flush writes a full 32-byte FIFO tail, including finish marker if requested.
 {
  std::lock_guard<std::mutex> lock(writerMutex);
  if(!__GDCurrentDL)throw std::logic_error("No native GX command buffer");
  writer=__GDCurrentDL;originalEnd=writer->ptr;
  GDOverflowCheck(32+(finish?5:0));
  if(finish)GDWriteBPCmd(0x45000002);
  for(unsigned i=0;i<32;++i)GDWrite_u8(0);
  batch.assign(writer->start,writer->ptr);
  Native::notifyGxCommandObserver(writer->start,batch.size());
  // The submitter owns a stable copy now. Reset before submission so an alarm
  // callback can safely construct its recovery list while this batch executes.
  writer->ptr=writer->start;
  Native::notifyGxCommandObserver(nullptr,0,true);
 }
 try{backend->submit(batch.data(),batch.size(),finish);}catch(...){
  std::lock_guard<std::mutex> lock(writerMutex);
  if(writer->ptr==writer->start)writer->ptr=originalEnd;
  throw;
 }
 completionPending.store(true,std::memory_order_release);
}
}
namespace Native {
AsyncGxCommands::AsyncGxCommands():previous(__GDCurrentDL),active(previous==nullptr){
 if(active){storage.resize(1024*1024);GDInitGDLObj(&writer,storage.data(),u32(storage.size()));GDSetCurrent(&writer);}
}
AsyncGxCommands::~AsyncGxCommands(){if(active)GDSetCurrent(previous);}
void AsyncGxCommands::commit(){
 if(!active)return;
 if(__GDCurrentDL!=&writer)throw std::logic_error("Async GX job lost its command writer");
 storage.resize(GDGetGDLObjOffset(&writer));
 if(!storage.empty()){std::lock_guard<std::mutex> lock(asyncCommandsMutex);asyncCommands.push_back(std::move(storage));}
 GDSetCurrent(previous);active=false;
}
void drainAsyncGxCommands(){
 std::lock_guard<std::mutex> lock(asyncCommandsMutex);
 size_t total=0;for(const auto& commands:asyncCommands){if(commands.size()>UINT32_MAX-total)throw std::length_error("Async GX commands exceed frame capacity");total+=commands.size();}
 if(!total)return;
 GDOverflowCheck(u32(total));
 for(const auto& commands:asyncCommands){std::memcpy(__GDCurrentDL->ptr,commands.data(),commands.size());__GDCurrentDL->ptr+=commands.size();}
 asyncCommands.clear();
}
bool gxCommandCompletionPending() noexcept{return completionPending.load(std::memory_order_acquire);}
void waitForGxAbortRecovery(){
 std::unique_lock<std::mutex> lock(abortRecoveryMutex);
 if(!abortRecoveryChanged.wait_for(lock,std::chrono::seconds(5),[]{return !abortRecoveryPending||abortRecoveryReady;})){
  abortRecoveryPending=false;abortRecoveryReady=false;
  throw std::runtime_error("Timed out waiting for the GX abort recovery acknowledgement");
 }
 abortRecoveryPending=false;abortRecoveryReady=false;
}
void markGxAbortRecoveryReady() noexcept{
 {std::lock_guard<std::mutex> lock(abortRecoveryMutex);if(!abortRecoveryPending)return;abortRecoveryReady=true;}
 abortRecoveryChanged.notify_all();
}
void appendGxAbortResetCommand(){
 std::lock_guard<std::mutex> lock(writerMutex);
 if(!__GDCurrentDL)throw std::logic_error("No native GX command buffer");
 GDOverflowCheck(5);GDWrite_u8(0x61);GDWrite_u32(0x5800000f);
}
CommandSubmissionBinding::CommandSubmissionBinding(CommandSubmitter& value):previous(backend){backend=&value;}
CommandSubmissionBinding::~CommandSubmissionBinding(){backend=previous;}
}
extern "C" void GXFlush(){flush(false);}
extern "C" void GXSetDrawDone(){flush(true);}
extern "C" void GXWaitDrawDone(){if(!backend)throw std::logic_error("No native GX command submitter");backend->waitForFinish();Native::waitForGxAbortRecovery();completionPending.store(false,std::memory_order_release);}
extern "C" void GXDrawDone(){GXSetDrawDone();GXWaitDrawDone();}

extern "C" void GXAbortFrame(){
 {std::lock_guard<std::mutex> lock(abortRecoveryMutex);abortRecoveryPending=true;abortRecoveryReady=false;}
 if(backend)backend->abortFrame();
 std::lock_guard<std::mutex> lock(writerMutex);
 if(__GDCurrentDL){__GDCurrentDL->ptr=__GDCurrentDL->start;Native::notifyGxCommandObserver(nullptr,0,true);}
 gx->dirtyState=0;gx->bpSentNot=GX_FALSE;
}



extern "C" void GXSetDrawSync(u16 token){
 if(!backend)throw std::logic_error("No native GX command submitter");
 if(!__GDCurrentDL)throw std::logic_error("No native GX command buffer");
 // Original GXMisc writes PE_TOKEN_INT followed by PE_TOKEN, then the FIFO tail.
 // Reserve the whole sequence before changing the writer so rejection is atomic.
 GDOverflowCheck(42);
 auto* writer=__GDCurrentDL;auto* originalEnd=writer->ptr;
 GDWriteBPCmd(0x48000000u|token);
 GDWriteBPCmd(0x47000000u|token);
 try{flush(false);}catch(...){writer->ptr=originalEnd;throw;}
}
