#include <JSystem/JKernel/JKRThread.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <cassert>
#include <cstdio>
int main(){alignas(32) unsigned char storage[131072]{};JKRExpHeap parent(storage,sizeof(storage),nullptr,false);JKRHeap::sRootHeap=&parent;parent.becomeCurrentHeap();auto initial=parent.getTotalFreeSize();{JKRThread thread(&parent,4096,16,16);assert(parent.do_getSize(thread.mMesgBuffer)>=16*sizeof(OSMessage));for(uintptr_t i=1;i<=16;++i)assert(OSSendMessage(&thread.mMessageQueue,reinterpret_cast<OSMessage>(i),OS_MESSAGE_NOBLOCK));assert(!OSSendMessage(&thread.mMessageQueue,nullptr,OS_MESSAGE_NOBLOCK));for(uintptr_t i=1;i<=16;++i){OSMessage value;assert(OSReceiveMessage(&thread.mMessageQueue,&value,OS_MESSAGE_NOBLOCK));assert(reinterpret_cast<uintptr_t>(value)==i);}assert(parent.check());OSResumeThread(thread.getThreadRecord());while(!OSIsThreadTerminated(thread.getThreadRecord()))OSYieldThread();}assert(parent.check());assert(parent.getTotalFreeSize()==initial);assert(JKRThread::sThreadList.getNumLinks()==0);std::puts("PASS original thread native queue capacity, pointer messages, detached completion and heap restoration");}


