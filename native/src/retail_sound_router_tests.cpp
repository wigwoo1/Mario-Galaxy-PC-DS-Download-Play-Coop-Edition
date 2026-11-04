#define WIN32_LEAN_AND_MEAN
#include "assets.hpp"
#include "native_dvd_port.hpp"
#include <JSystem/JAudio2/JAUAudioArcInterpreter.hpp>
#include <JSystem/JAudio2/JAISeMgr.hpp>
#include <JSystem/JAudio2/JAUSoundMgr.hpp>
#include <JSystem/JAudio2/JAISeq.hpp>
#include <JSystem/JAudio2/JAIStream.hpp>
#include <JSystem/JAudio2/JAUStreamDataMgr.hpp>
#include <Native/JasStreamEndian.hpp>
#include <JSystem/JAudio2/JAUSeqCollection.hpp>
#include <JSystem/JAudio2/JAUStdSoundInfo.hpp>
#include <JSystem/JAudio2/JAUSoundTable.hpp>
#include <JSystem/JAudio2/JAISoundChild.hpp>
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
JASWaveBank* expectedWaveBank=nullptr;JAISeMgr* manager=nullptr;JAISoundHandle* soundHandle=nullptr;
unsigned cycles=0,cycleNonzero=0,programMask=0;JAISeqDataRegion expectedRegion{};u32 archiveBase=0,archiveLength=0;
JAUSoundMgr* router=nullptr;JAISoundHandle* streamHandle=nullptr;JAUStreamStaticAramMgr* streamAram=nullptr;unsigned overlaps=0,maxSource=0;std::chrono::steady_clock::time_point streamStart;
void inspect(JASTrack* track){
 if(!track)return;
 for(u32 m=0;m<track->mChannelMgrCount;++m)if(auto* mgr=track->mChannelMgrs[m])for(auto* ch:mgr->mChannels)if(ch){
  ++active;assert(ch->mChannelType==JASChannel::CH_WAVE&&ch->mBankDisposeID);
  auto* bank=static_cast<const JASBank*>(ch->mBankDisposeID);assert(bank->getWaveBank()==expectedWaveBank);
  assert(track->mBankNumber==64&&(track->mProgNumber==10||track->mProgNumber==11));programMask|=1u<<track->mProgNumber;
  const u32 at=u32(ch->mWavePtr);assert(at>=archiveBase&&u64(at)+ch->mWaveInfo.mAWLength<=u64(archiveBase)+archiveLength);++voiceChecks;
  auto* dsp=ch->mDspCh?ch->mDspCh->mChannel:nullptr;
  if(dsp&&dsp->isActive()){assert(u32(dsp->_118)==u32(ch->mWavePtr)&&dsp->_11C==ch->mWaveInfo.mSampleCount&&dsp->_102==ch->mWaveInfo.mLoopFlags);++dspWaveChecks;}
 }
 for(auto* child:track->mChildren)if(child)inspect(child);
}
s32 monitor(void*){
 assert(OSGetCurrentThread()!=mainThread);std::lock_guard<std::mutex> lock(mutex);++subframes;
 if(subframes%7==0){router->calc();router->mixOut();}
 assert(router->getSeqMgr().getNumActiveSeqs()==0);
 auto* stream=streamHandle->isSoundAttached()?streamHandle->getSound()->asStream():nullptr;unsigned streamVoices=0;
 if(stream){for(auto* channel:stream->inner_.aramStream.mChannels)if(channel)++streamVoices;maxSource=std::max(maxSource,u32(stream->inner_.aramStream._0B8));}
 if(streamVoices&&soundHandle->isSoundAttached())++overlaps;
 live=soundHandle->isSoundAttached()?soundHandle->getSound()->getTrack():nullptr;inspect(live);
 if(live&&live->mSeqCtrl.mReader.mSeqBuff)assert(manager->isUsingSeqData(expectedRegion));
 const bool drained=!soundHandle->isSoundAttached()&&manager->getNumActiveSe()==0&&JASTrack::sTrackList.mTracks.size()==0&&JASDSPChannel::getNumFree()==64-streamVoices&&JASChannel::memPool_.getFreeMemCount()==72-streamVoices;
 if(drained&&cycles<3){
  assert(JAISe::memPool_.getFreeMemCount()==120&&JASTrack::memPool_.getFreeMemCount()==200&&JASTrack::TChannelMgr::memPool_.getFreeMemCount()==1&&JAISoundChild::memPool_.getFreeMemCount()==160-(stream?2:0));
  assert(!manager->isUsingSeqData(expectedRegion)&&nonzero>cycleNonzero);++cycles;
  std::printf("PASS natural coin cycle%u: handle detached, region unused, SE/track pools restored while concurrent stream keeps its own children/channels\n",cycles);
  if(cycles<3){cycleNonzero=nonzero;assert(JAISoundStarter::getInstance()->startSound(JAISoundID(0),soundHandle,nullptr));assert(soundHandle->isSoundAttached());}
  
 }
 finished=cycles==3&&!streamHandle->isSoundAttached()&&router->getNumActiveSounds()==0&&streamAram->mBits.none()&&JASDSPChannel::getNumFree()==64&&JASChannel::memPool_.getFreeMemCount()==72;
 changed.notify_all();return 0;
}
void pcm(s16* data,u32 frames){
 assert(OSGetCurrentThread()!=mainThread&&frames==560);
 std::lock_guard<std::mutex> lock(mutex);++dacFrames;
 for(unsigned i=0;i<frames*2;++i)nonzero+=data[i]!=0;
 changed.notify_all();
}
void dumpTrack(const JASTrack* t,unsigned depth=0){
 if(!t||depth>3)return;
 std::printf("COIN track=%p depth=%u bank=%u program=%u bankObject=%p status=%u seqOffset=%u\n",static_cast<const void*>(t),depth,unsigned(t->mBankNumber),unsigned(t->mProgNumber),t->mBankTable?static_cast<void*>(t->mBankTable->getBank(t->mBankNumber)):nullptr,unsigned(t->mStatus),t->mSeqCtrl.mReader.mSeqCursor?u32(t->mSeqCtrl.mReader.mSeqCursor-t->mSeqCtrl.mReader.mSeqBuff):0);
 for(auto* child:t->mChildren)if(child)dumpTrack(child,depth+1);
}
LONG WINAPI observe(EXCEPTION_POINTERS* p){
 if(p->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION){dumpTrack(live);
 std::printf("ACCESS VIOLATION op=%llu address=%p instruction=%p module=%p\n",static_cast<unsigned long long>(p->ExceptionRecord->ExceptionInformation[0]),reinterpret_cast<void*>(p->ExceptionRecord->ExceptionInformation[1]),p->ExceptionRecord->ExceptionAddress,GetModuleHandle(nullptr));
 auto process=GetCurrentProcess();SymSetOptions(SYMOPT_LOAD_LINES|SYMOPT_UNDNAME);SymInitialize(process,nullptr,TRUE);auto context=*p->ContextRecord;STACKFRAME64 frame{};frame.AddrPC.Offset=context.Rip;frame.AddrPC.Mode=AddrModeFlat;frame.AddrStack.Offset=context.Rsp;frame.AddrStack.Mode=AddrModeFlat;frame.AddrFrame.Offset=context.Rbp;frame.AddrFrame.Mode=AddrModeFlat;
 for(unsigned i=0;i<12;++i){alignas(SYMBOL_INFO) char buffer[sizeof(SYMBOL_INFO)+1024]{};auto* sym=reinterpret_cast<SYMBOL_INFO*>(buffer);sym->SizeOfStruct=sizeof(SYMBOL_INFO);sym->MaxNameLen=1024;DWORD64 delta=0;IMAGEHLP_LINE64 line{};line.SizeOfStruct=sizeof(line);DWORD displacement=0;SymFromAddr(process,frame.AddrPC.Offset,&delta,sym);SymGetLineFromAddr64(process,frame.AddrPC.Offset,&displacement,&line);std::printf("%p %s +%llu %s:%lu\n",reinterpret_cast<void*>(frame.AddrPC.Offset),sym->Name,static_cast<unsigned long long>(delta),line.FileName?line.FileName:"?",line.LineNumber);if(!StackWalk64(IMAGE_FILE_MACHINE_AMD64,process,GetCurrentThread(),&frame,&context,nullptr,SymFunctionTableAccess64,SymGetModuleBase64,nullptr))break;}
 }
 return EXCEPTION_CONTINUE_SEARCH;
}
struct Catalog:JAUAudioArcInterpreter {
 const u8* wave=nullptr;const u8* bank=nullptr;std::array<const u8*,256> waves{};const u8* bst=nullptr;const u8* bstn=nullptr;const u8* bsc=nullptr;u32 bstBytes=0,bscBytes=0;u32 waveIndex=0;unsigned waveCount=0,bankCount=0;
 void readWS(u32 id,const void* p,u32)override{assert(id<waves.size());++waveCount;waves[id]=static_cast<const u8*>(p);}
 void readBNK(u32 ws,const void* p)override{++bankCount;if(JASBNKParser::getBankNumber(p)==64){assert(!bank&&ws<waves.size());bank=static_cast<const u8*>(p);waveIndex=ws;wave=waves[ws];}}
 void readBSC(const void* p,u32 n)override{bsc=static_cast<const u8*>(p);bscBytes=n;}void readBST(const void* p,u32 n)override{bst=static_cast<const u8*>(p);bstBytes=n;}void readBSTN(const void* p,u32)override{bstn=static_cast<const u8*>(p);}void readBMS(u32,const void*,u32)override{}void readBMS_fromArchive(u32)override{}void newVoiceBank(u32,u32)override{}void newDynamicSeqBlock(u32)override{}void readBSFT(const void*)override{}void readMaxSeCategory(int,int,int)override{}void beginBNKList(u32,u32)override{}void endBNKList()override{}bool readCommandMore(u32)override{assert(false);return false;}
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
 auto* arc=waveBank->getWaveArc(0);assert(arc&&arc->mEntryNum==DVDConvertPathToEntrynum("/AudioRes/Waves/B64kawa_0.aw"));
 std::vector<u8> expected;assert(nativeReadDvdEntry(arc->mEntryNum,expected));
 std::vector<u8> streamBytes;const int streamEntry=DVDConvertPathToEntrynum("/AudioRes/Stream/SMG_ev_prolo04_strm.ast");assert(streamEntry>=0&&nativeReadDvdEntry(streamEntry,streamBytes));JASAramStream::Header streamHeader{};assert(Native::decodeJasStreamHeader(streamBytes.data(),streamBytes.size(),streamHeader)&&streamHeader.loop_end==841863&&streamHeader.channels==2&&!streamHeader.loop);
 std::vector<u8> aramBytes(expected.size()+0x400000,0xcd);auto aram=std::make_shared<NativeAramMemory>(aramBytes.data(),aramBytes.size());assert(nativeBindAramTransport(aram));
 {
 JASHeap waveRoot;waveRoot.initRootHeap(reinterpret_cast<void*>(uintptr_t(0x4000)),u32((expected.size()+31)&~size_t(31)));
 JASHeap streamRoot;const u32 streamBase=u32((0x5000+expected.size()+31)&~size_t(31));streamRoot.initRootHeap(reinterpret_cast<void*>(uintptr_t(streamBase)),0x300000);JAUStreamStaticAramMgr streamMemory;streamAram=&streamMemory;
 auto* streamWorker=JKRAramStream::create(16);assert(streamWorker);assert(JASDvd::createThread(16,64,0x4000));auto* dvdWorker=JASDvd::getThreadPointer();assert(dvdWorker);
 assert(arc->load(&waveRoot));
 auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(10);
 while(true){{JASMutexLock lock(&arc->mMutex);if(arc->mStatus==2&&arc->_5A==0)break;}assert(std::chrono::steady_clock::now()<deadline);std::this_thread::yield();}
 const auto waveBase=uintptr_t(arc->getHeap()->getBase());assert(waveBase>=0x4000&&waveBase+expected.size()<=aramBytes.size());assert(std::equal(expected.begin(),expected.end(),aramBytes.begin()+waveBase));archiveBase=u32(waveBase);archiveLength=u32(expected.size());
 std::printf("PASS bank64 waveform-system2 archive B64kawa_0.aw exact transfer: %u bytes at%x\n",archiveLength,archiveBase);
 expectedWaveBank=waveBank;JASTrack::sDefaultBankTable.registBank(bankNumber,bank);
 JAUSoundTable table(true);table.init(catalog.bst);table.nativeResourceSize=catalog.bstBytes;JAUSoundNameTable names(true);names.init(catalog.bstn);JAUStdSoundInfo info(true);
 JAUSeqDataMgr_SeqCollection collection;collection.init(catalog.bsc);assert(collection.isValid()&&collection.getSeqDataRegion(&expectedRegion)&&expectedRegion.addr==catalog.bsc&&expectedRegion.size<=catalog.bscBytes);
 const auto groupOffset=be32(catalog.bsc+8),seqOffset=be32(catalog.bsc+groupOffset+4);extent(groupOffset,8,catalog.bscBytes);extent(seqOffset,1,catalog.bscBytes);
 JAISeqData seq(nullptr,0);assert(collection.getSeqData(JAISoundID(0),&seq)==2&&seq.data==catalog.bsc&&seq.offset==seqOffset&&seqOffset==0x411);
 assert(!std::strcmp(names.getName(JAISoundID(0)),"SE_SY_COIN"));std::printf("Actual catalog ID00000000 %s; BSC bytes=%u offset=%x priority=%u\n",names.getName(JAISoundID(0)),catalog.bscBytes,seq.offset,info.getPriority(JAISoundID(0)));
 JAIStream::newMemPool(3);JAISeq::newMemPool(3);JASAramStream::initSystem(streamHeader.block_size,2);
 // Allocation uses the initialized static block size, not an implicit zero.
 assert(JASAramStream::getBlockSize()==streamHeader.block_size&&streamHeader.block_size>0);
 streamMemory.alloc(36,&streamRoot);assert(streamMemory.mSize==2);
 for(const auto& heap:streamMemory.mHeaps){const auto base=uintptr_t(heap.mBase);assert(heap.mSize==36*streamHeader.block_size&&heap.mSize>0&&base>=streamBase&&base+heap.mSize<=aramBytes.size());}
 std::printf("PASS stream system initialized before ARAM allocation: block=%u buffers=%u bytesEach=%u; disjoint from loaded coin wave archive\n",streamHeader.block_size,streamMemory.mSize,streamMemory.mHeaps[0].mSize);

 JAISe::newMemPool(120);JASTrack::newMemPool(200);JASTrack::TChannelMgr::newMemPool(1);JAISoundChild::newMemPool(160);
 DSPInit();Native::nativeDspAddressSpace().clear();JASDsp::setDSPMixerLevel(.25f);
 {
 JAUSoundMgr system;router=&system;auto& seManager=system.getSeMgr();manager=&seManager;seManager.setSeqDataMgr(&collection);
 system.getStreamMgr().setStreamDataMgr(&info);system.getStreamMgr().setStreamAramMgr(&streamMemory);
 assert(JAISoundStarter::getInstance()==&system&&JAISeMgr::getInstance()==&seManager&&JAISeqMgr::getInstance()==&system.getSeqMgr()&&JAIStreamMgr::getInstance()==&system.getStreamMgr());
 JAISoundHandle handle,prologue;soundHandle=&handle;streamHandle=&prologue;
 assert(!JAISoundStarter::getInstance()->startSound(JAISoundID(0xff000000),nullptr,nullptr)&&system.getNumActiveSounds()==0);
 assert(JAISoundStarter::getInstance()->startSound(JAISoundID(0),&handle,nullptr));assert(handle.isSoundAttached());
 const u8* bst=catalog.bst;const u32 bstRoot=be32(bst+12),section=be32(bst+bstRoot+4),group=be32(bst+section+4),descriptor=be32(bst+group+8),record=descriptor&0xffffff;
 extent(record,2,catalog.bstBytes);assert(table.getData(JAISoundID(0))==reinterpret_cast<const JAUSoundTableItem*>(bst+record));
 assert(info.getSoundType(JAISoundID(0))==0&&info.getCategory(JAISoundID(0))==0&&info.getPriority(JAISoundID(0))==bst[record]);
 assert(handle.getSound()->asSe()->inner_.mProperPriority==bst[record]);assert(std::fabs(handle.getSound()->mParams.mProperty.mVolume-float(bst[record+1])/255.0f)<.000001f);
 std::printf("PASS actual coin catalog descriptor=%08x priority=%u authored volume=%u/255 BSC offset=%x\n",descriptor,bst[record],bst[record+1],seqOffset);
 assert(JAISoundStarter::getInstance()->startSound(JAISoundID(0x02000022),&prologue,nullptr));assert(prologue.isSoundAttached()&&prologue.getSound()->asStream()&&!prologue.getSound()->asSe());
 assert(std::fabs(prologue.getSound()->mParams.mProperty.mVolume-85.0f/255.0f)<.000001f);assert(system.getNumActiveSounds()==2&&seManager.getNumActiveSe()==1&&system.getStreamMgr().getNumActiveStreams()==1);
 streamStart=std::chrono::steady_clock::now();
 assert(JASDriver::registerDspSyncCallback(monitor,nullptr));JASDriver::dacCallbackFunc=pcm;
 JASAudioThread::create(16);auto* audio=JASAudioThread::getThreadPointer();assert(audio);
 {
 std::unique_lock<std::mutex> lock(mutex);
 bool pass=changed.wait_for(lock,std::chrono::seconds(40),[]{return finished&&active>0&&nonzero>0&&dacFrames>=30;});
 std::printf("actual sound router: subframes=%u active=%u dac=%u nonzero=%u finish=%u trackStatus=%u parentsFree=%u dspFree=%d\n",subframes,active,dacFrames,nonzero,unsigned(finished),live?unsigned(live->mStatus):99,JASChannel::memPool_.getFreeMemCount(),JASDSPChannel::getNumFree());assert(pass&&voiceChecks>0&&dspWaveChecks>0&&programMask==((1u<<10)|(1u<<11))&&overlaps>0&&maxSource>800000);
 const double elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-streamStart).count();assert(elapsed>=.95*(841863.0/32000)&&elapsed<=1.10*(841863.0/32000));
 assert(JAIStream::memPool_.getFreeMemCount()==3&&JAISeq::memPool_.getFreeMemCount()==3&&JAISoundChild::memPool_.getFreeMemCount()==160);
 std::printf("PASS real starter routes coin and prologue simultaneously; overlapSubframes=%u maxStreamSource=%u naturalSeconds=%.3f\n",overlaps,maxSource,elapsed);
 std::printf("PASS actual coin voice identity/metadata checks=%u; actual DSP ADPCM source/count/loop/block checks=%u\n",voiceChecks,dspWaveChecks);
 }
 audio->stop();deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);
 while(!OSIsThreadTerminated(audio->getThreadRecord())&&std::chrono::steady_clock::now()<deadline)std::this_thread::yield();
 assert(OSIsThreadTerminated(audio->getThreadRecord())&&!nativeDspAudioRuntimeActive());
 assert(JASDriver::rejectCallback(monitor,nullptr)==1);assert(JASDriver::rejectCallback(JASTrack::TList::cbSeqMain,&JASTrack::sTrackList)==1);JASTrack::sTrackList.mCallbackRegistered=false;
 seManager.resetSeqDataMgr();
 JASDriver::dacCallbackFunc=nullptr;AIRegisterDMACallback(nullptr);audio->~JASAudioThread();JKRHeap::free(audio,dram);
 }
 JASTrack::sDefaultBankTable.registBank(bankNumber,nullptr);assert(arc->erase());
 dvdWorker->~JASTaskThread();JKRHeap::free(dvdWorker,dram);
 static_cast<JKRThread*>(streamWorker)->~JKRThread();JKRHeap::free(streamWorker,nullptr);
 assert(!JASAudioThread::getThreadPointer()&&!JASDvd::getThreadPointer()&&JKRThread::sThreadList.getNumLinks()==0);
 static_cast<JASSimpleWaveBank*>(waveBank)->~JASSimpleWaveBank();JKRHeap::free(waveBank,bankHeap);
 JKRHeap::destroy(bankHeap);
 } // Stream ARAM child heaps unlink while their stack root remains alive.
 streamAram=nullptr;Native::nativeDspAddressSpace().clear();nativeReleaseAramTransport();JKRHeap::destroy(dram);
 assert(!JASDram&&root.check()&&root.getTotalFreeSize()==freeBefore);
 }
 assert(VirtualFree(memory,0,MEM_RELEASE));nativeReleaseDvd();
 std::puts("PASS original JAISoundStarter/JAUSoundMgr routes actual coin and prologue; SE_SY_COIN catalog/collection/JAISe request -> original retail sequence -> game track/bank/channel/audio/DSP/native AI/DAC; natural finish/manager and heap cleanup");
}
}
int main(int argc,char** argv){assert(argc==2);std::setvbuf(stdout,nullptr,_IONBF,0);auto* observer=AddVectoredExceptionHandler(1,observe);assert(observer);noteWorker(argv[1]);assert(RemoveVectoredExceptionHandler(observer));}
