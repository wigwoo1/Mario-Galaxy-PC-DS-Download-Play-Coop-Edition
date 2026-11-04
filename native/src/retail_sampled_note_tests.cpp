#define WIN32_LEAN_AND_MEAN
#include "assets.hpp"
#include "native_dvd_port.hpp"
#include <JSystem/JAudio2/JAUAudioArcInterpreter.hpp>
#include <JSystem/JAudio2/JASBNKParser.hpp>
#include <JSystem/JAudio2/JASWSParser.hpp>
#include <JSystem/JAudio2/JASBankTable.hpp>
#include <JSystem/JAudio2/JASBasicBank.hpp>
#include <JSystem/JAudio2/JASWaveArcLoader.hpp>
#include <JSystem/JAudio2/JASSimpleWaveBank.hpp>
#include <JSystem/JAudio2/JASDvdThread.hpp>
#include <JSystem/JAudio2/JASMutex.hpp>
#include <JSystem/JAudio2/JASTaskThread.hpp>
#include <JSystem/JKernel/JKRAramStream.hpp>
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
unsigned subframes=0,dacFrames=0,nonzero=0,active=0,voiceChecks=0,dspWaveChecks=0;bool finished=false;
JASBank* expectedBank=nullptr;JASWaveInfo expectedInfo;u32 expectedWaveAddress=0;
s32 monitor(void*){
 assert(OSGetCurrentThread()!=mainThread);
 std::lock_guard<std::mutex> lock(mutex);++subframes;
 if(live->getChannelCount()){
  ++active;auto* ch=live->mChannelMgrs[0]->mChannels[1];assert(ch);
  assert(ch->mChannelType==JASChannel::CH_WAVE&&ch->mBankDisposeID==expectedBank&&u32(ch->mWavePtr)==expectedWaveAddress);
  assert(ch->mWaveInfo.mFormat==expectedInfo.mFormat&&ch->mWaveInfo.mSampleCount==expectedInfo.mSampleCount&&ch->mWaveInfo.mLoopFlags==expectedInfo.mLoopFlags);++voiceChecks;
  auto* dsp=ch->mDspCh?ch->mDspCh->mChannel:nullptr;
  if(dsp&&dsp->isActive()){
   assert(u32(dsp->_118)==expectedWaveAddress&&dsp->_11C==expectedInfo.mSampleCount&&dsp->_102==expectedInfo.mLoopFlags);
   if(!dspWaveChecks)std::printf("DSP block fields: samples=%u bytes=%u format=%u\n",unsigned(dsp->_64),unsigned(dsp->_100),expectedInfo.mFormat);// _64 is the live remaining decoded AFC buffer count after DSP service,
   // not a fixed block size; original renderer consumes it in16-sample blocks.
   assert(dsp->_64<=16&&dsp->_100==9);++dspWaveChecks;
  }
 }
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
struct Catalog:JAUAudioArcInterpreter {
 const u8* wave=nullptr;const u8* bank=nullptr;u32 waveIndex=0;unsigned waveCount=0,bankCount=0;
 void readWS(u32 id,const void* p,u32)override{++waveCount;if(id==0)wave=static_cast<const u8*>(p);}
 void readBNK(u32 ws,const void* p)override{++bankCount;if(ws==0){assert(!bank);bank=static_cast<const u8*>(p);waveIndex=ws;}}
 void readBSC(const void*,u32)override{}void readBST(const void*,u32)override{}void readBSTN(const void*,u32)override{}void readBMS(u32,const void*,u32)override{}void readBMS_fromArchive(u32)override{}void newVoiceBank(u32,u32)override{}void newDynamicSeqBlock(u32)override{}void readBSFT(const void*)override{}void readMaxSeCategory(int,int,int)override{}void beginBNKList(u32,u32)override{}void endBNKList()override{}bool readCommandMore(u32)override{assert(false);return false;}
};
void noteWorker(const char* wbfs){
 mainThread=OSGetCurrentThread();assert(nativeBindDvd(wbfs));
 std::vector<u8> packed;const int smr=DVDConvertPathToEntrynum("/AudioRes/SMR.szs");assert(smr>=0&&nativeReadDvdEntry(smr,packed));auto bytes=decodeYaz0(packed);
 Catalog catalog;assert(bytes.size()==596320&&catalog.parse(bytes.data(),bytes.size())&&catalog.wave&&catalog.bank&&catalog.waveCount==48&&catalog.bankCount==48);
 const u32 bankNumber=JASBNKParser::getBankNumber(catalog.bank);assert(bankNumber<256);
 std::printf("Retail catalog: %u banks/%u waveform systems; selected bank=%u waveSystem=%u\n",catalog.bankCount,catalog.waveCount,bankNumber,catalog.waveIndex);
 void* memory=VirtualAlloc(nullptr,0x800000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);assert(memory);
 {
 JKRExpHeap root(memory,0x800000,nullptr,false);JKRHeap::sRootHeap=JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();
 const auto freeBefore=root.getTotalFreeSize();auto* dram=JKRSolidHeap::create(0x500000,&root,false);assert(dram);JASKernel::setupRootHeap(dram,0x200000);
 auto* bankHeap=JKRExpHeap::create(0x100000,&root,false);assert(bankHeap);
 auto* bank=JASBNKParser::Ver1::createBasicBank(catalog.bank,bankHeap);assert(bank);
 auto* waveBank=JASWSParser::createWaveBank(catalog.wave,bankHeap);assert(waveBank&&waveBank->getArcCount()==1);bank->assignWaveBank(waveBank);
 auto* arc=waveBank->getWaveArc(0);assert(arc&&arc->mEntryNum==DVDConvertPathToEntrynum("/AudioRes/Waves/B00mahit_0.aw"));
 std::vector<u8> expected;assert(nativeReadDvdEntry(arc->mEntryNum,expected));
 std::vector<u8> aramBytes(expected.size()+0x10000,0xcd);auto aram=std::make_shared<NativeAramMemory>(aramBytes.data(),aramBytes.size());assert(nativeBindAramTransport(aram));
 JASHeap waveRoot;waveRoot.initRootHeap(reinterpret_cast<void*>(uintptr_t(0x4000)),u32(aramBytes.size()-0x4000));
 auto* streamWorker=JKRAramStream::create(16);assert(streamWorker);assert(JASDvd::createThread(16,64,0x4000));auto* dvdWorker=JASDvd::getThreadPointer();assert(dvdWorker);
 assert(arc->load(&waveRoot));
 auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(10);
 while(true){{JASMutexLock lock(&arc->mMutex);if(arc->mStatus==2&&arc->_5A==0)break;}assert(std::chrono::steady_clock::now()<deadline);std::this_thread::yield();}
 const auto waveBase=uintptr_t(arc->getHeap()->getBase());assert(waveBase>=0x4000&&waveBase+expected.size()<=aramBytes.size());assert(std::equal(expected.begin(),expected.end(),aramBytes.begin()+waveBase));
 unsigned program=240,key=60,velocity=100;JASInstParam params;
 for(unsigned p=0;p<240;++p)if(bank->getInstParam(p,key,velocity,&params)&&params.mChannelType==JASChannel::CH_WAVE&&waveBank->getWaveHandle(params._18)){program=p;break;}
 assert(program<240);auto* handle=waveBank->getWaveHandle(params._18);auto* info=handle->getWaveInfo();assert(info&&handle->getWavePtr()&&*info->_24);
 std::printf("Actual loaded bank%u program%u key%u velocity%u wave%u format%u samples%d loop%u pointer=%x archiveBytes=%zu\n",bankNumber,program,key,velocity,params._18,info->mFormat,info->mSampleCount,info->mLoopFlags,handle->getWavePtr(),expected.size());
 expectedBank=bank;expectedInfo=*info;expectedWaveAddress=u32(handle->getWavePtr());
 JASTrack::sDefaultBankTable.registBank(bankNumber,bank);
 DSPInit();Native::nativeDspAddressSpace().clear();JASDsp::setDSPMixerLevel(.25f);
 {
 JASTrack track;live=&track;
 u8 sequence[]={0xe2,u8(bankNumber),0xe3,u8(program),u8(key),1,u8(velocity),0xf1,20,0x81,0xf1,20,0xff};
 track.setSeqData(sequence,0);track.startSeq();
 assert(JASDriver::registerDspSyncCallback(monitor,nullptr));JASDriver::dacCallbackFunc=pcm;
 JASAudioThread::create(16);auto* audio=JASAudioThread::getThreadPointer();assert(audio);
 {
 std::unique_lock<std::mutex> lock(mutex);
 bool pass=changed.wait_for(lock,std::chrono::seconds(8),[]{return finished&&active>0&&nonzero>0&&dacFrames>=30;});
 std::printf("actual sampled note: subframes=%u active=%u dac=%u nonzero=%u finish=%u trackStatus=%u parentsFree=%u dspFree=%d\n",subframes,active,dacFrames,nonzero,unsigned(finished),unsigned(track.mStatus),JASChannel::memPool_.getFreeMemCount(),JASDSPChannel::getNumFree());assert(pass&&voiceChecks>0&&dspWaveChecks>0);
 std::printf("PASS sampled voice identity/metadata checks=%u; actual DSP ADPCM source/count/loop/block checks=%u\n",voiceChecks,dspWaveChecks);
 }
 audio->stop();deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);
 while(!OSIsThreadTerminated(audio->getThreadRecord())&&std::chrono::steady_clock::now()<deadline)std::this_thread::yield();
 assert(OSIsThreadTerminated(audio->getThreadRecord())&&!nativeDspAudioRuntimeActive());
 assert(JASDriver::rejectCallback(monitor,nullptr)==1);assert(JASDriver::rejectCallback(JASTrack::TList::cbSeqMain,&JASTrack::sTrackList)==1);JASTrack::sTrackList.mCallbackRegistered=false;
 JASDriver::dacCallbackFunc=nullptr;AIRegisterDMACallback(nullptr);audio->~JASAudioThread();JKRHeap::free(audio,dram);
 }
 JASTrack::sDefaultBankTable.registBank(bankNumber,nullptr);assert(arc->erase());
 dvdWorker->~JASTaskThread();JKRHeap::free(dvdWorker,dram);
 static_cast<JKRThread*>(streamWorker)->~JKRThread();JKRHeap::free(streamWorker,nullptr);
 assert(!JASAudioThread::getThreadPointer()&&!JASDvd::getThreadPointer()&&JKRThread::sThreadList.getNumLinks()==0);
 static_cast<JASSimpleWaveBank*>(waveBank)->~JASSimpleWaveBank();JKRHeap::free(waveBank,bankHeap);
 JKRHeap::destroy(bankHeap);Native::nativeDspAddressSpace().clear();nativeReleaseAramTransport();JKRHeap::destroy(dram);
 assert(!JASDram&&root.check()&&root.getTotalFreeSize()==freeBefore);
 }
 assert(VirtualFree(memory,0,MEM_RELEASE));nativeReleaseDvd();
 std::puts("PASS retail BAA bank/wave construction, original DVD/ARAM waveform load and exact bytes, actual sequence sampled note -> game track/bank/channel -> original audio/DSP/native AI/DAC; note-off/finish, worker/heap cleanup and normal return");
}
}
int main(int argc,char** argv){assert(argc==2);std::setvbuf(stdout,nullptr,_IONBF,0);auto* observer=AddVectoredExceptionHandler(1,observe);assert(observer);noteWorker(argv[1]);assert(RemoveVectoredExceptionHandler(observer));}
