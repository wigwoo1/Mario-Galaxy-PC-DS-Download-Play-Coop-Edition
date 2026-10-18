#include <Native/HeapPlacement.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JAudio2/JASBasicInst.hpp>
#include <cassert>
#include <JSystem/JUtility/JUTException.hpp>
#include <cstdio>
#include <cstdarg>
#include <cstring>
static void* expectedSolidFrees[4]{}; static unsigned solidFreeWarnings=0;
extern "C" void JUTWarningConsole_f(const char* format,...){va_list args;va_start(args,format);if(std::strcmp(format,"free: cannot free memory block (%08x)\n")==0){void* ptr=va_arg(args,void*);for(auto& expected:expectedSolidFrees){if(expected&&expected==ptr){expected=nullptr;++solidFreeWarnings;va_end(args);return;}}va_end(args);std::fprintf(stderr,"Unexpected solid free %p\n",ptr);assert(false);}std::vfprintf(stderr,format,args);va_end(args);std::fflush(stderr);assert(false);}
extern "C" void JUTReportConsole_f(const char*,...){}
extern "C" void JUTWarningConsole(const char* message){std::fputs(message,stderr);assert(false);}
extern "C" void JUTReportConsole(const char* message){std::fputs(message,stderr);}
void JUTException::panic_f(const char* file,int line,const char* format,...){std::fprintf(stderr,"%s:%d ",file,line);va_list args;va_start(args,format);std::vfprintf(stderr,format,args);va_end(args);assert(false);}
#include "native_dvd_port.hpp"
#include "native_jkr_aram_port.hpp"
#include <JSystem/JKernel/JKRDvdFile.hpp>
#include <JSystem/JKernel/JKRAramStream.hpp>
#include <JSystem/JKernel/JKRDvdAramRipper.hpp>
#include <JSystem/JSupport/JSUFileStream.hpp>
#include <revolution/dvd.h>
#include <vector>
#include <atomic>
#include <chrono>
#include <thread>
#include <JSystem/JAudio2/JASWaveArcLoader.hpp>
#include <JSystem/JAudio2/JASDvdThread.hpp>
#include <JSystem/JAudio2/JASTaskThread.hpp>
struct WaveLoadProbe: JASWaveArc {std::atomic<unsigned> loads{0},erases{0};void onLoadDone()override{++loads;}void onEraseDone()override{++erases;}};
static void taskBarrier(void* p){static_cast<std::atomic<bool>*>(p)->store(true);}
int main(int argc,char**argv){if(argc!=2)return 2;std::setvbuf(stdout,nullptr,_IONBF,0);std::puts("Transfer begin");assert(nativeBindDvd(argv[1]));std::puts("Transfer DVD ready");int entry=DVDConvertPathToEntrynum("/AudioRes/Waves/B00mahit_0.aw");assert(entry>=0);std::vector<uint8_t> expected;assert(nativeReadDvdEntry(entry,expected));unsigned fileLength=expected.size();unsigned length=(expected.size()+31)&~31u;std::vector<uint8_t> backing(length+0x8000,0xcd);auto memory=std::make_shared<NativeAramMemory>(backing.data(),backing.size());assert(nativeBindAramTransport(memory));alignas(32) static u8 arena[262144];JKRSolidHeap parent(arena,sizeof(arena),nullptr,false);JKRHeap::sRootHeap=&parent;auto* heap=JKRExpHeap::create(196608,&parent,false);assert(heap);heap->becomeCurrentHeap();JKRHeap::sSystemHeap=heap;auto before=heap->getTotalFreeSize();std::puts("Transfer heap ready");auto* worker=JKRAramStream::create(16);assert(worker);{JKRDvdFile file;assert(file.open(entry));JSUFileInputStream stream(&file);JKRAramStreamCommand command;command.mStream=&stream;command.mSize=length;command.mOffset=0;command.mAddress=0x4000;command.mTransferBuffer=nullptr;command.mTransferBufferSize=4096;command.mHeap=heap;u32 returned=0;command.mReturnSize=&returned;OSInitMessageQueue(&command.mMessageQueue,&command.mMessage,1);std::puts("Transfer stream ready");auto result=JKRAramStream::writeToAram(&command);std::printf("Transfer returned %d expected %u reported %u\n",result,length,returned);assert(result==length&&returned==length);OSMessage done=nullptr;assert(OSReceiveMessage(&command.mMessageQueue,&done,OS_MESSAGE_NOBLOCK));assert(reinterpret_cast<uintptr_t>(done)==length);assert(std::equal(expected.begin(),expected.end(),backing.begin()+0x4000));std::puts("Transfer data exact");for(unsigned i=0;i<0x4000;++i)assert(backing[i]==0xcd);assert(backing[0x4000+length]==0xcd);std::puts("Transfer guards intact");std::fill(backing.begin()+0x4000,backing.begin()+0x4000+length,0xcc);
JKRAramStream::setTransBuffer(nullptr,4096,heap);u32 queuedSize=0;
auto* queued=JKRAramStream::write_StreamToAram_Async(&stream,0x4000,length,0,&queuedSize);
assert(queued&&JKRAramStream::sync(queued,0)==queued);assert(queuedSize==length);
assert(std::equal(expected.begin(),expected.end(),backing.begin()+0x4000));
queued->~JKRAramStreamCommand();JKRHeap::free(queued,nullptr);std::puts("Queued transfer exact and command freed");std::fill(backing.begin()+0x4000,backing.begin()+0x4000+length,0xaa);u32 ripSize=0;
auto* rip=JKRDvdAramRipper::loadToAram(entry,0x4000,EXPAND_SWITCH_UNKNOWN0,0,0,&ripSize);
assert(rip&&ripSize==length);assert(!JKRDvdAramRipper::loadToAram(-1,0x4000,EXPAND_SWITCH_UNKNOWN0,0,0,nullptr));assert(std::equal(expected.begin(),expected.end(),backing.begin()+0x4000));
assert(JKRDvdAramRipper::sDvdAramAsyncList.getNumLinks()==0);std::puts("Original DVD ARAM loader exact and command list empty");auto* audioHeap=JKRSolidHeap::create(32768,heap,false);assert(audioHeap);JASDram=audioHeap;JASKernel::sSystemHeap=heap;
std::puts("Wave pool construct");JASTaskThread::ThreadMemPool taskPool;std::puts("Wave pool ready");JASKernel::sCommandHeap=&taskPool;
std::puts("Wave DVD thread create");assert(JASDvd::createThread(16,8,0x4000));auto* task=JASDvd::getThreadPointer();std::puts("Wave DVD thread ready");
{JASHeap root;root.initRootHeap(reinterpret_cast<void*>(uintptr_t(0x4000)),length+0x4000);std::puts("Wave root ready");WaveLoadProbe wave;std::puts("Wave object ready");wave.setFileName("B00mahit_0.aw");assert(wave.mEntryNum==entry);std::puts("Wave filename ready");assert(wave.load(&root));std::puts("Wave load submitted");auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(10);while(!wave.loads&&std::chrono::steady_clock::now()<deadline)std::this_thread::yield();std::printf("Wave load count %u status %d pending %u\n",wave.loads.load(),wave.mStatus,wave._5A);assert(wave.loads==1);assert(wave.mStatus==2&&wave._48==1&&wave._5A==0);assert(std::equal(expected.begin(),expected.end(),backing.begin()+uintptr_t(wave.getHeap()->getBase())));assert(wave.erase());assert(wave.erases==1&&wave.mStatus==0);std::puts("Wave erase callback and unloaded state verified");}
std::atomic<bool> barrier{false};assert(task->sendCmdMsg(taskBarrier,&barrier));while(!barrier)std::this_thread::yield();OSCancelThread(task->mThread);std::puts("Wave task stopped; destroying solid-heap task");expectedSolidFrees[0]=task->mStackMemory;expectedSolidFrees[1]=task->mThread;expectedSolidFrees[2]=task->mMesgBuffer;expectedSolidFrees[3]=task;task->~JASTaskThread();JKRHeap::free(task,nullptr);assert(solidFreeWarnings==4);for(auto ptr:expectedSolidFrees)assert(!ptr);JASDram=nullptr;audioHeap->~JKRSolidHeap();JKRHeap::free(audioHeap,heap);
for(auto* chunk=taskPool.mChunk;chunk;){auto* next=chunk->mNextChunk;chunk->~MemoryChunk();JKRHeap::free(chunk,nullptr);chunk=next;}taskPool.mChunk=nullptr;JASKernel::sCommandHeap=nullptr;std::puts("Original waveform task load/erase callbacks passed");}std::puts("Transfer file closed");static_cast<JKRThread*>(worker)->~JKRThread();JKRHeap::free(worker,nullptr);std::puts("Transfer worker stopped");auto* restarted=JKRAramStream::create(16);assert(restarted);static_cast<JKRThread*>(restarted)->~JKRThread();JKRHeap::free(restarted,nullptr);assert(JKRThread::sThreadList.getNumLinks()==0);std::puts("Transfer worker restart cleaned");std::printf("Transfer heap check %d free %u before %u\n",heap->check(),heap->getTotalFreeSize(),before);assert(heap->check()&&heap->getTotalFreeSize()==before);nativeReleaseAramTransport();nativeReleaseDvd();std::printf("PASS original stream worker retail archive: %u bytes, completion, exact data, bounds and buffer heap restoration\n",length);}
















