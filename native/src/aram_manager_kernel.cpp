#include <JSystem/JKernel/JKRAram.hpp>
#include <JSystem/JKernel/JKRAramPiece.hpp>
#include <Native/HeapPlacement.hpp>
#include <stdexcept>
// Retail US constructor 0x803a5604, stack/message globals 0x8069dbdc/0x8069dbe0.
JKRAram* JKRAram::sAramObject=nullptr;
OSMessage JKRAram::sMessageBuffer[4]{};
OSMessageQueue JKRAram::sMessageQueue{};
OSMutex JKRAramPiece::mMutex{};
JKRAram::JKRAram(u32 audio,u32 graph,s32 priority):JKRThread(0xc000,32,priority){
 u32 base=ARInit(mStackArray,3),size=ARGetSize();
 if(size<base || audio>size-base)throw std::out_of_range("ARAM audio partition exceeds backing");
 mAudioMemorySize=audio;
 if(graph==0xffffffffu){mGraphMemorySize=size-audio-base;mAramMemorySize=0;}
 else{if(graph>size-base-audio)throw std::out_of_range("ARAM graph partition exceeds backing");mGraphMemorySize=graph;mAramMemorySize=size-audio-graph-base;}
 mAudioMemoryPtr=ARAlloc(mAudioMemorySize);mGraphMemoryPtr=ARAlloc(mGraphMemorySize);
 mAramMemoryPtr=mAramMemorySize?ARAlloc(mAramMemorySize):0;
 // Native command worker replaces Wii ARQ hardware scheduling.
 OSInitMutex(&JKRAramPiece::mMutex);
 mAramHeap=new (JKRGetSystemHeap(),0) JKRAramHeap(mGraphMemoryPtr,mGraphMemorySize);
 OSInitMessageQueue(&sMessageQueue,sMessageBuffer,4);
}
JKRAram::~JKRAram(){
 if(!OSIsThreadTerminated(mThread))OSCancelThread(mThread);
 if(sAramObject==this)sAramObject=nullptr;
 delete mAramHeap;
}
void* JKRAram::run(){
 
 for(;;){OSMessage msg;OSReceiveMessage(&sMessageQueue,&msg,OS_MESSAGE_BLOCK);auto message=static_cast<JKRAramCommand*>(msg);int kind=message->field_0x00;auto command=static_cast<JKRAMCommand*>(message->command);message->~JKRAramCommand();JKRHeap::free(message,nullptr);if(kind==1)JKRAramPiece::startDMA(command);}
}


