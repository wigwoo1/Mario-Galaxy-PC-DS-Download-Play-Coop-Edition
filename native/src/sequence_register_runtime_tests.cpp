#include <Native/HeapPlacement.hpp>
#include <JSystem/JAudio2/JASTrack.hpp>
#include <JSystem/JAudio2/JASSeqParser.hpp>
#include <JSystem/JAudio2/JASDriverIF.hpp>
#include <array>
#include <cassert>
#include <cstdio>
#include <vector>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#include <Native/DspAddressSpace.hpp>
#include <Native/DspAudioRuntime.hpp>
#include <JSystem/JAudio2/JASAiCtrl.hpp>
#include <JSystem/JAudio2/JASAudioThread.hpp>
#include <JSystem/JAudio2/JASDSPChannel.hpp>
#include <JSystem/JAudio2/JASHeapCtrl.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <JSystem/JKernel/JKRSolidHeap.hpp>
#include <revolution/ai.h>
#include <revolution/dsp.h>
#include "native_jkr_aram_port.hpp"
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>


namespace {
std::mutex mutex;std::condition_variable changed;
JASTrack* live=nullptr;OSThread* mainThread=nullptr;
unsigned subframes=0,dacFrames=0,nonzero=0,active=0;bool finished=false;
s32 monitor(void*){
 assert(OSGetCurrentThread()!=mainThread);
 std::lock_guard<std::mutex> lock(mutex);++subframes;
 if(live->getChannelCount())++active;
 finished=live->mStatus==JASTrack::STATUS_STOPPED&&live->sTrackList.mTracks.size()==0&&JASDSPChannel::getNumFree()==64&&JASChannel::memPool_.getFreeMemCount()==72;
 changed.notify_all();return 0;
}
void pcm(s16* data,u32 frames){
 assert(OSGetCurrentThread()!=mainThread&&frames==560);
 std::lock_guard<std::mutex> lock(mutex);++dacFrames;
 for(unsigned i=0;i<frames*2;++i)nonzero+=data[i]!=0;
 changed.notify_all();
}
LONG WINAPI observe(EXCEPTION_POINTERS* p){
 if(p->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION){
 std::printf("ACCESS VIOLATION op=%llu address=%p instruction=%p module=%p\n",static_cast<unsigned long long>(p->ExceptionRecord->ExceptionInformation[0]),reinterpret_cast<void*>(p->ExceptionRecord->ExceptionInformation[1]),p->ExceptionRecord->ExceptionAddress,GetModuleHandle(nullptr));
 auto process=GetCurrentProcess();SymSetOptions(SYMOPT_LOAD_LINES|SYMOPT_UNDNAME);SymInitialize(process,nullptr,TRUE);auto context=*p->ContextRecord;STACKFRAME64 frame{};frame.AddrPC.Offset=context.Rip;frame.AddrPC.Mode=AddrModeFlat;frame.AddrStack.Offset=context.Rsp;frame.AddrStack.Mode=AddrModeFlat;frame.AddrFrame.Offset=context.Rbp;frame.AddrFrame.Mode=AddrModeFlat;
 for(unsigned i=0;i<12;++i){alignas(SYMBOL_INFO) char buffer[sizeof(SYMBOL_INFO)+1024]{};auto* sym=reinterpret_cast<SYMBOL_INFO*>(buffer);sym->SizeOfStruct=sizeof(SYMBOL_INFO);sym->MaxNameLen=1024;DWORD64 delta=0;IMAGEHLP_LINE64 line{};line.SizeOfStruct=sizeof(line);DWORD displacement=0;SymFromAddr(process,frame.AddrPC.Offset,&delta,sym);SymGetLineFromAddr64(process,frame.AddrPC.Offset,&displacement,&line);std::printf("%p %s +%llu %s:%lu\n",reinterpret_cast<void*>(frame.AddrPC.Offset),sym->Name,static_cast<unsigned long long>(delta),line.FileName?line.FileName:"?",line.LineNumber);if(!StackWalk64(IMAGE_FILE_MACHINE_AMD64,process,GetCurrentThread(),&frame,&context,nullptr,SymFunctionTableAccess64,SymGetModuleBase64,nullptr))break;}
 }
 return EXCEPTION_CONTINUE_SEARCH;
}
void noteWorker(){
 mainThread=OSGetCurrentThread();
 void* memory=VirtualAlloc(nullptr,0x400000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);assert(memory);
 {
 JKRExpHeap root(memory,0x400000,nullptr,false);JKRHeap::sRootHeap=JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();
 const auto freeBefore=root.getTotalFreeSize();JASDram=JKRSolidHeap::create(0x300000,&root,false);assert(JASDram);
 std::vector<u8> aramBytes(0x10000);auto aram=std::make_shared<NativeAramMemory>(aramBytes.data(),aramBytes.size());assert(nativeBindAramTransport(aram));
 DSPInit();Native::nativeDspAddressSpace().clear();JASDsp::setDSPMixerLevel(.25f);
 {
 JASTrack track;live=&track;
 // Default original programF0 selects the engine's oscillator voice. No
 // proprietary bank/wave fixture and no synthetic channel callback override.
 u8 sequence[]={60,1,127,0xf1,20,0x81,0xf1,20,0xff};
 track.setSeqData(sequence,0);track.startSeq();
 assert(JASDriver::registerDspSyncCallback(monitor,nullptr));JASDriver::dacCallbackFunc=pcm;
 JASAudioThread::create(16);auto* audio=JASAudioThread::getThreadPointer();assert(audio);
 {
 std::unique_lock<std::mutex> lock(mutex);
 bool pass=changed.wait_for(lock,std::chrono::seconds(8),[]{return finished&&active>0&&nonzero>0&&dacFrames>=30;});
 std::printf("actual sequence note: subframes=%u active=%u dac=%u nonzero=%u finish=%u trackStatus=%u parentsFree=%u dspFree=%d\n",subframes,active,dacFrames,nonzero,unsigned(finished),unsigned(track.mStatus),JASChannel::memPool_.getFreeMemCount(),JASDSPChannel::getNumFree());
 assert(pass);
 }
 audio->stop();auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);
 while(!OSIsThreadTerminated(audio->getThreadRecord())&&std::chrono::steady_clock::now()<deadline)std::this_thread::yield();
 assert(OSIsThreadTerminated(audio->getThreadRecord())&&!nativeDspAudioRuntimeActive());
 assert(JASDriver::rejectCallback(monitor,nullptr)==1);
 assert(JASDriver::rejectCallback(JASTrack::TList::cbSeqMain,&JASTrack::sTrackList)==1);JASTrack::sTrackList.mCallbackRegistered=false;
 JASDriver::dacCallbackFunc=nullptr;AIRegisterDMACallback(nullptr);
 audio->~JASAudioThread();JKRHeap::free(audio,JASDram);
 assert(!JASAudioThread::getThreadPointer()&&JKRThread::sThreadList.getNumLinks()==0);
 }
 Native::nativeDspAddressSpace().clear();nativeReleaseAramTransport();JKRHeap::destroy(JASDram);
 assert(!JASDram&&root.check()&&root.getTotalFreeSize()==freeBefore);
 }
 assert(VirtualFree(memory,0,MEM_RELEASE));
 std::puts("PASS actual sequence bytecode -> original track list/controller/parser -> game channelStart -> bank oscillator -> original audio/DSP/native AI/DAC; note-off/finish and complete heap cleanup, normal return");
}
}
int main(){
 std::setvbuf(stdout,nullptr,_IONBF,0);
 JASTrack track;
 std::vector<u8> bytes={0xd8,0x20,0xab,0xcd,0xf1,2,0xd8,4,0x12,0x34,0xff};
 track.setSeqData(bytes.data(),0);
 assert(track.tickProc()==0);
 assert(track.mRegisterParam._0[0]==0xab&&track.mRegisterParam._0[1]==0xcd&&track.mSeqCtrl.mTimer==2);
 assert(track.tickProc()==0&&track.mSeqCtrl.mTimer==1);
 assert(track.tickProc()==-1&&track.mRegisterParam._0[4]==0x1234);
 assert(track.mSeqCtrl.mReader.mNumStacks==0);
 std::puts("PASS original sequence parser/control/track register32 load, two-tick wait, BE16 load and finish");

 // Loop commands are original parser operations with BE16 operands.
 {JASTrack t;u8 loop[]={0xd8,4,0,0,0xcb,0,3,0xda,1,4,0,2,0xcc,0xf1,1,0xff};t.setSeqData(loop,0);assert(t.tickProc()==0&&t.mRegisterParam._0[4]==6&&t.mSeqCtrl.mReader.mNumStacks==0);assert(t.tickProc()==-1);}
 // A genuine24-bit call/return executes a register-add subroutine, then
 // resumes the caller and consumes its own later BE16 operand.
 {JASTrack t;u8 call[]={0xd8,4,1,0,0xc3,0,0,15,0xd8,5,0x22,0x22,0xf1,1,0xff,0xda,1,4,0,1,0xc5};t.setSeqData(call,0);assert(t.tickProc()==0&&t.mRegisterParam._0[4]==0x101&&t.mRegisterParam._0[5]==0x2222&&t.mSeqCtrl.mReader.mNumStacks==0);assert(t.tickProc()==-1);}
 // The parser's register-indirect argument mask must resolve destinationA=4.
 {JASTrack t;u8 indirect[]={0xd8,0,0,4,0x90,0x80,0xd8,0,0x12,0x34,0xf1,1,0xff};t.setSeqData(indirect,0);assert(t.tickProc()==0&&t.mRegisterParam._0[4]==0x1234);assert(t.tickProc()==-1);}
 std::puts("PASS original BE16 loop/add, BE24 call/return and register-indirect command operands; no custom parser/note overrides");
 auto* observer=AddVectoredExceptionHandler(1,observe);assert(observer);noteWorker();assert(RemoveVectoredExceptionHandler(observer));
}
