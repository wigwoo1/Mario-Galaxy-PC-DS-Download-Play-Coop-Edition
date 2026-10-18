#include <memory>
#include <cassert>
#include <cstdio>
#include <Game/Util/Functor.hpp>
#include <Game/System/FunctionAsyncExecutor.hpp>
struct Receiver {int value=0;OSPriority seen=-1;void execute(int delta,int* output){seen=OSGetThreadPriority(OSGetCurrentThread());value+=delta;*output=value;}};
void* workerEntry(void* argument){auto job=static_cast<FunctionAsyncExecInfo*>(argument);assert(OSGetThreadPriority(OSGetCurrentThread())==7);job->execute();assert(OSGetThreadPriority(OSGetCurrentThread())==7);job->mIsEnd=true;assert(OSSendMessage(&job->mQueue,nullptr,OS_MESSAGE_BLOCK));return argument;}
void* dispatchEntry(void* argument){static_cast<FunctionAsyncExecutorOnMainThread*>(argument)->update();return nullptr;}
int main(){auto current=OSGetCurrentThread();assert(OSSetThreadPriority(current,18));assert(!OSSetThreadPriority(current,-1));assert(!OSSetThreadPriority(current,32));Receiver receiver;int output=0;auto callback=MR::Functor(&receiver,&Receiver::execute,5,&output);FunctionAsyncExecInfo job(callback.clone(nullptr),4,"NativeJob");assert(!job.mIsEnd);job.execute();assert(output==5 && receiver.seen==4 && OSGetThreadPriority(current)==18);
 FunctionAsyncExecutorOnMainThread executor(current);assert(OSSendMessage(&executor.mQueue,&job,OS_MESSAGE_NOBLOCK));executor.update();assert(job.mIsEnd && receiver.value==10 && receiver.seen==18 && OSGetThreadPriority(current)==18);OSMessage completed=reinterpret_cast<void*>(1);assert(OSReceiveMessage(&job.mQueue,&completed,OS_MESSAGE_NOBLOCK));assert(completed==nullptr);executor.update();assert(receiver.value==10); FunctionAsyncExecInfo threaded(callback.clone(nullptr),4,"NativeWorkerJob");OSThread worker;char stack[4096];assert(OSCreateThread(&worker,workerEntry,&threaded,stack+sizeof(stack),sizeof(stack),7,0));assert(OSResumeThread(&worker)==1);while(!threaded.mIsEnd.load(std::memory_order_acquire))OSYieldThread();assert(receiver.value==15 && output==15);assert(OSReceiveMessage(&threaded.mQueue,&completed,OS_MESSAGE_BLOCK));assert(completed==nullptr);void* result=nullptr;assert(OSJoinThread(&worker,&result));assert(result==&threaded && threaded.mIsEnd && receiver.value==15 && receiver.seen==4);assert(OSGetThreadPriority(current)==18);
 for(int iteration=0;iteration<100;++iteration){
  auto pending=new FunctionAsyncExecInfo(callback.clone(nullptr),4,"CompletionLifetime");
  assert(OSSendMessage(&executor.mQueue,pending,OS_MESSAGE_NOBLOCK));
  assert(OSCreateThread(&worker,dispatchEntry,&executor,stack+sizeof(stack),sizeof(stack),7,0));
  assert(OSResumeThread(&worker)==1);
  assert(OSReceiveMessage(&pending->mQueue,&completed,OS_MESSAGE_BLOCK));
  assert(pending->mIsEnd && completed==nullptr);
  delete pending; // Notification must be the executor's last access to this job.
  assert(OSJoinThread(&worker,nullptr));
 }
 assert(receiver.value==515);
 std::puts("PASS original async job priority restoration, main-thread dispatch, completion queue and immediate job reclamation");}



