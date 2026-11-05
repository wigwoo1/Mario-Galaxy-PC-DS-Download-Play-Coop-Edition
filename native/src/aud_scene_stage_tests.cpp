#define WIN32_LEAN_AND_MEAN
#include "assets.hpp"
#include "native_dvd_port.hpp"
#include <JSystem/JAudio2/JAUAudioArcInterpreter.hpp>
#include <JSystem/JAudio2/JAISeMgr.hpp>
#include <JSystem/JAudio2/JAUSoundMgr.hpp>
#include <JSystem/JAudio2/JAISeq.hpp>
#include <JSystem/JAudio2/JAUSectionHeap.hpp>
#include <JSystem/JKernel/JKRDvdArchive.hpp>
#include <JSystem/JKernel/JKRMemArchive.hpp>
#include <JSystem/JKernel/JKRAram.hpp>
#include <JSystem/JKernel/JKRAramBlock.hpp>
#include <JSystem/JKernel/JKRFileLoader.hpp>
#include <bitset>
#include <optional>
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
#include "native_jkr_archive_port.hpp"
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>


#include <JSystem/JAudio2/JAUAudioArcLoader.hpp>
#include <Game/AudioLib/AudSystem.hpp>
#include <Game/AudioLib/AudSceneMgr.hpp>
#include <Game/AudioLib/AudSeStrategy.hpp>
#include <JSystem/JAudio2/JAISoundStarter.hpp>
#include <JSystem/JAudio2/JASCriticalSection.hpp>
#include <atomic>
#include <Native/AudioAiDiagnostics.hpp>
#include <Native/VideoBinding.hpp>
#include <Native/VideoRetraceScheduler.hpp>
#include <Game/AudioLib/AudParams.hpp>
#include <Game/RhythmLib/AudRhythmMeSystem.hpp>
#include <Game/AudioLib/AudEffector.hpp>
#include <Game/AudioLib/AudSoundObject.hpp>
#include <Game/RhythmLib/AudMePlayer.hpp>
#include <Game/RhythmLib/AudMeObject.hpp>
#include <Game/AudioLib/AudRemixMgr.hpp>
#include <Game/Speaker/SpkSystem.hpp>
#include <Game/Speaker/SpkSpeakerCtrl.hpp>
#include <JSystem/JAudio2/JAUStreamDataMgr.hpp>
#include <JSystem/JAudio2/JASBasicWaveBank.hpp>

namespace {
std::atomic<bool> playbackReady{false},tailDrainRequested{false},tailsRetired{false};
std::atomic<unsigned> subframes{0},dacChunks{0},nonzeroSamples{0};
OSThread* testMainThread=nullptr;
s32 playbackMonitor(void*){
 assert(OSGetCurrentThread()!=testMainThread);
 if(playbackReady.load(std::memory_order_acquire))subframes.fetch_add(1,std::memory_order_relaxed);
 // Only after the main thread has finished all sound-manager requests, observe
 // retirement on the audio worker that owns these voice/track updates.
 if(tailDrainRequested.load(std::memory_order_acquire)&&JASDSPChannel::getNumFree()==64&&JASChannel::memPool_.getFreeMemCount()==72&&JASTrack::sTrackList.mTracks.size()==0&&JASTrack::memPool_.getFreeMemCount()==200&&JASTrack::TChannelMgr::memPool_.getFreeMemCount()==1)tailsRetired.store(true,std::memory_order_release);

 return 0;
}
void playbackPcm(s16* samples,u32 frames){
 assert(OSGetCurrentThread()!=testMainThread&&frames==560);
 unsigned count=0;for(unsigned i=0;i<frames*2;++i)count+=samples[i]!=0;
 nonzeroSamples.fetch_add(count,std::memory_order_relaxed);dacChunks.fetch_add(1,std::memory_order_relaxed);
}
LONG WINAPI observe(EXCEPTION_POINTERS* p){
 if(p->ExceptionRecord->ExceptionCode==EXCEPTION_ACCESS_VIOLATION){
 std::printf("ACCESS VIOLATION op=%llu address=%p instruction=%p module=%p\n",static_cast<unsigned long long>(p->ExceptionRecord->ExceptionInformation[0]),reinterpret_cast<void*>(p->ExceptionRecord->ExceptionInformation[1]),p->ExceptionRecord->ExceptionAddress,GetModuleHandle(nullptr));
 auto process=GetCurrentProcess();SymSetOptions(SYMOPT_LOAD_LINES|SYMOPT_UNDNAME);SymInitialize(process,nullptr,TRUE);auto context=*p->ContextRecord;STACKFRAME64 frame{};frame.AddrPC.Offset=context.Rip;frame.AddrPC.Mode=AddrModeFlat;frame.AddrStack.Offset=context.Rsp;frame.AddrStack.Mode=AddrModeFlat;frame.AddrFrame.Offset=context.Rbp;frame.AddrFrame.Mode=AddrModeFlat;
 for(unsigned i=0;i<12;++i){alignas(SYMBOL_INFO) char buffer[sizeof(SYMBOL_INFO)+1024]{};auto* sym=reinterpret_cast<SYMBOL_INFO*>(buffer);sym->SizeOfStruct=sizeof(SYMBOL_INFO);sym->MaxNameLen=1024;DWORD64 delta=0;IMAGEHLP_LINE64 line{};line.SizeOfStruct=sizeof(line);DWORD displacement=0;SymFromAddr(process,frame.AddrPC.Offset,&delta,sym);SymGetLineFromAddr64(process,frame.AddrPC.Offset,&displacement,&line);std::printf("%p %s +%llu %s:%lu\n",reinterpret_cast<void*>(frame.AddrPC.Offset),sym->Name,static_cast<unsigned long long>(delta),line.FileName?line.FileName:"?",line.LineNumber);if(!StackWalk64(IMAGE_FILE_MACHINE_AMD64,process,GetCurrentThread(),&frame,&context,nullptr,SymFunctionTableAccess64,SymGetModuleBase64,nullptr))break;}
 }
 return EXCEPTION_CONTINUE_SEARCH;
}
struct Inventory:JAUAudioArcInterpreter {
 struct Bank {u32 id,ws;};std::vector<Bank> banks;std::array<u32,255> masks{};unsigned waveCount=0;const u8* bst=nullptr;u32 bstSize=0;const u8* names=nullptr;u32 nameSize=0;const u8* bsc=nullptr;u32 bscSize=0;
 void readWS(u32 id,const void*,u32 mask)override{assert(id<255);masks[id]=mask;++waveCount;}
 void readBNK(u32 ws,const void* p)override{banks.push_back({JASBNKParser::getBankNumber(p),ws});}
 void readBST(const void* p,u32 n)override{bst=static_cast<const u8*>(p);bstSize=n;}
 void readBSTN(const void* p,u32 n)override{names=static_cast<const u8*>(p);nameSize=n;}
 void readBSC(const void* p,u32 n)override{bsc=static_cast<const u8*>(p);bscSize=n;}
 void readBMS(u32,const void*,u32)override{assert(false);}void readBMS_fromArchive(u32)override{assert(false);}void newVoiceBank(u32,u32)override{assert(false);}void newDynamicSeqBlock(u32)override{assert(false);}void readBSFT(const void*)override{assert(false);}void readMaxSeCategory(int,int,int)override{assert(false);}void beginBNKList(u32,u32)override{assert(false);}void endBNKList()override{assert(false);}bool readCommandMore(u32)override{assert(false);return false;}
};
unsigned verifiedMemoryResources=0;
void verifyMemoryArchive(JKRArchive& archive,const char* path){
 std::vector<u8> raw;assert(nativeReadDvdEntry(DVDConvertPathToEntrynum(path),raw));Rarc expected(raw);
 assert(archive.mIsMounted&&archive.mMountMode==JKRArchive::MOUNT_MODE_MEM&&nativeOwnedRarcTables(&archive));
 assert(archive.countResource()==expected.files.size());
 for(const auto& entry:expected.files){
  auto bytes=expected.readFile(entry.path);void* actual=archive.getResource(entry.path.c_str());
  assert(actual&&archive.getResSize(actual)==bytes.size()&&(!bytes.size()||std::memcmp(actual,bytes.data(),bytes.size())==0));++verifiedMemoryResources;
 }
}
void run(const char* path,bool preloaded){
 testMainThread=OSGetCurrentThread();assert(testMainThread);
 Native::VideoInterface video;Native::VideoBinding videoBinding(video);VIInit();
 Native::VideoRetraceScheduler videoScheduler(video,std::chrono::nanoseconds(16666667));

 assert(nativeBindDvd(path));std::vector<u8> packed;assert(nativeReadDvdEntry(DVDConvertPathToEntrynum("/AudioRes/SMR.szs"),packed));auto reference=decodeYaz0(packed);Inventory inventory;assert(reference.size()==596320&&inventory.parse(reference.data(),reference.size()));
 void* memory=VirtualAlloc(nullptr,0x2000000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);assert(memory);
 {
 JKRExpHeap root(memory,0x2000000,nullptr,false);JKRHeap::sRootHeap=JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();const auto initial=root.getTotalFreeSize();
 auto* platformHeap=JKRExpHeap::create(0x200000,&root,false);assert(platformHeap);JKRHeap::sSystemHeap=platformHeap;platformHeap->becomeCurrentHeap();
 const auto mountsFree=platformHeap->getTotalFreeSize();
 for(const char* archivePath:{"/AudioRes/Info/JaiChord.arc","/AudioRes/Info/JaiMe.arc"})
 for(auto direction:{JKRArchive::MOUNT_DIRECTION_1,static_cast<JKRArchive::EMountDirection>(0)}){
  {JKRMemArchive archive(DVDConvertPathToEntrynum(archivePath),direction);verifyMemoryArchive(archive,archivePath);}
  assert(platformHeap->check()&&platformHeap->getTotalFreeSize()==mountsFree&&JKRFileLoader::sVolumeList.getNumLinks()==0);
 }
 {
  std::vector<u8> source;assert(nativeReadDvdEntry(DVDConvertPathToEntrynum("/AudioRes/Info/JaiChord.arc"),source));
  JKRMemArchive archive;assert(nativeRegisterRarc(source.data(),source.size())&&archive.mountFixed(source.data(),JKR_MEM_BREAK_FLAG_0));
  nativeUnregisterRarc(source.data());std::fill(source.begin(),source.end(),0xa5);source.clear();source.shrink_to_fit();
  verifyMemoryArchive(archive,"/AudioRes/Info/JaiChord.arc");
 }
 assert(platformHeap->check()&&platformHeap->getTotalFreeSize()==mountsFree&&JKRFileLoader::sVolumeList.getNumLinks()==0);
 std::printf("PASS memory-archive resources: %u exact files, both allocation directions; old buffer mount survives source overwrite/release\n",verifiedMemoryResources);
 std::puts("CHECK non-RARC file rejected by public memory-archive mount");
 const auto beforeRejected=platformHeap->getTotalFreeSize();
 assert(!JKRArchive::mount("/AudioRes/SMR.szs",JKRArchive::MOUNT_MODE_MEM,platformHeap,JKRArchive::MOUNT_DIRECTION_1));
 assert(platformHeap->check()&&platformHeap->getTotalFreeSize()==beforeRejected&&JKRFileLoader::sVolumeList.getNumLinks()==0);
 std::puts("PASS rejected non-RARC mount restores allocation heap and volume list");
 auto* dram=JKRSolidHeap::create(0x1800000,&root,false);assert(dram);
 std::vector<u8> aramBytes(0x1000000,0xcd);auto aram=std::make_shared<NativeAramMemory>(aramBytes.data(),aramBytes.size());assert(nativeBindAramTransport(aram));
 auto* manager=new(platformHeap,alignof(JKRAram))JKRAram(AudParams::aramSize,0xffffffffu,16);assert(manager);JKRAram::sAramObject=manager;
 assert(manager->getAudioMemory()==0x4000&&manager->getAudioMemSize()==0xE00000);
 auto* aramWorker=JKRAramStream::create(16);assert(aramWorker);
 DSPInit();Native::nativeDspAddressSpace().clear();JASDriver::dacCallbackFunc=playbackPcm;assert(JASDriver::registerDspSyncCallback(playbackMonitor,nullptr));
 {
 std::optional<JKRDvdArchive> remixArchive;
 JKRArchive* suppliedSeq=nullptr;JKRArchive* suppliedChord=nullptr;JKRArchive* suppliedMe=nullptr;JKRArchive* suppliedRemix=nullptr;
 auto borrowedCatalog=reference;
 if(preloaded){
  auto* savedHeap=dram->becomeCurrentHeap();
  suppliedSeq=JKRArchive::mount("/AudioRes/Seqs/JaiSeq.arc",JKRArchive::MOUNT_MODE_MEM,dram,JKRArchive::MOUNT_DIRECTION_1);
  suppliedChord=JKRArchive::mount("/AudioRes/Info/JaiChord.arc",JKRArchive::MOUNT_MODE_MEM,dram,JKRArchive::MOUNT_DIRECTION_1);
  suppliedMe=JKRArchive::mount("/AudioRes/Info/JaiMe.arc",JKRArchive::MOUNT_MODE_MEM,dram,JKRArchive::MOUNT_DIRECTION_1);
  suppliedRemix=JKRArchive::mount("/AudioRes/Info/JaiRemixSeq.arc",JKRArchive::MOUNT_MODE_MEM,dram,JKRArchive::MOUNT_DIRECTION_1);
  assert(suppliedSeq&&suppliedChord&&suppliedMe&&suppliedRemix);savedHeap->becomeCurrentHeap();
 }else{
  remixArchive.emplace(DVDConvertPathToEntrynum("/AudioRes/Info/JaiRemixSeq.arc"),JKRArchive::MOUNT_DIRECTION_1);assert(remixArchive->mIsMounted);suppliedRemix=&*remixArchive;
 }
 auto* priorHeap=JKRHeap::getCurrentHeap();
 std::printf("FULL FACTORY begin original AudNewAudSystem: %s\n",preloaded?"preloaded catalog and archives, matching game wrapper argument shape":"default catalog/sequence/chord/ME paths");
 auto* system=AudNewAudSystem(dram,preloaded?borrowedCatalog.data():nullptr,suppliedSeq,suppliedChord,suppliedMe,suppliedRemix);
 assert(system&&AudSystem::get()==system&&JKRHeap::getCurrentHeap()==priorHeap);
 auto& section=*system->mSectionHeap;auto* sectionHeap=static_cast<JKRSolidHeap*>(section.mHeap);
 auto* audioWorker=JASAudioThread::getThreadPointer();auto* dvdWorker=JASDvd::getThreadPointer();assert(audioWorker&&dvdWorker);
 const auto audioReadyDeadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
 while(!nativeDspAudioRuntimeActive()&&std::chrono::steady_clock::now()<audioReadyDeadline)std::this_thread::yield();
 assert(nativeDspAudioRuntimeActive()&&!OSIsThreadTerminated(audioWorker->getThreadRecord()));
 std::puts("PASS actual original audio worker initializes native DSP and remains alive");
 assert(JASDram==dram&&JASKernel::getSystemHeap()&&JASKernel::getSystemHeap()->mSize+((sizeof(JKRExpHeap)+15)&~size_t(15))==0x1000);
 assert(JASAramStream::getBlockSize()==0x2760&&JASAramStream::sChannelMax==4);
 assert(JAISe::memPool_.getTotalMemCount()==120&&JAISeq::memPool_.getTotalMemCount()==3&&JAIStream::memPool_.getTotalMemCount()==3&&JAISoundChild::memPool_.getTotalMemCount()==160);
 assert(JAUSectionHeap::getInstance()==&section&&!section.isBuilding()&&section.getHeap()==nullptr&&sectionHeap);
 auto* table=section.getSoundTable();auto* names=section.sectionHeapData_.soundNameTable;auto* collection=static_cast<JAUSeqDataMgr_SeqCollection*>(section.getSeSeqDataMgr());
 assert(table&&names&&collection&&JKRHeap::findFromRoot(system)==sectionHeap&&JKRHeap::findFromRoot(table)==sectionHeap);
 if(preloaded){
  std::fill(borrowedCatalog.begin(),borrowedCatalog.end(),0xa5);borrowedCatalog.clear();borrowedCatalog.shrink_to_fit();
  assert(std::memcmp(table->getResource(),inventory.bst,inventory.bstSize)==0);
  assert(std::memcmp(section.data_.mBstnDst,inventory.names,inventory.nameSize)==0);
  assert(std::memcmp(collection->getCollectionData(),inventory.bsc,inventory.bscSize)==0);
  std::puts("PASS preloaded catalog overwritten/released after factory; complete copied BST/BSTN/BSC remain exact");
 }
 assert(!std::strcmp(names->getName(JAISoundID(0)),"SE_SY_COIN")&&!std::strcmp(names->getName(JAISoundID(0x01000002)),"BGM_MISS"));
 auto& dynamic=section.sectionHeapData_.seqDataBlocks;
 assert(dynamic.getFreeBlocks().getNumLinks()==3&&dynamic.getLoadedBlocks().getNumLinks()==0);
 for(auto* link=dynamic.getFreeBlocks().getFirst();link;link=link->getNext()){
  auto* block=link->getObject();assert(JKRHeap::findFromRoot(block)==sectionHeap&&block->region.size==56000&&JKRHeap::findFromRoot(block->region.addr)==sectionHeap);
 }
 auto* seqArchive=dynamic.getSeqDataArchive();auto* chordArchive=system->mRhythmMeSystem->mChordInfo.mArchive;
 assert(seqArchive&&seqArchive->mMountMode==(preloaded?JKRArchive::MOUNT_MODE_MEM:JKRArchive::MOUNT_MODE_DVD)&&seqArchive->countResource()==105);
 assert(chordArchive&&chordArchive->mMountMode==JKRArchive::MOUNT_MODE_MEM&&chordArchive->countResource()==73);
 JKRArchive* meArchive=nullptr;
 for(auto* link=JKRFileLoader::sVolumeList.getFirst();link;link=link->getNext()){
  auto* archive=static_cast<JKRArchive*>(link->getObject());if(archive->mEntryNum==DVDConvertPathToEntrynum("/AudioRes/Info/JaiMe.arc"))meArchive=archive;
 }
 assert(meArchive&&meArchive->mMountMode==JKRArchive::MOUNT_MODE_MEM&&meArchive->countResource()==2);
 if(preloaded)assert(seqArchive==suppliedSeq&&chordArchive==suppliedChord&&meArchive==suppliedMe);
 for(auto* archive:{seqArchive,chordArchive,meArchive})assert(JKRHeap::findFromRoot(archive)==(preloaded?static_cast<JKRHeap*>(dram):sectionHeap));

 verifyMemoryArchive(*chordArchive,"/AudioRes/Info/JaiChord.arc");verifyMemoryArchive(*meArchive,"/AudioRes/Info/JaiMe.arc");
 assert(verifiedMemoryResources==298);
 for(const auto& heap:system->mStreamAramMgr->mHeaps)assert(heap.mSize==362880&&uintptr_t(heap.mBase)>=0x4000+119712&&uintptr_t(heap.mBase)+heap.mSize<=0xE04000);
 assert(system->mRhythmMeSystem->mMeMgr->mParams.mNumEntries==40&&system->mRemixMgr->mGroupCount==6&&system->mStreamAramMgr->mSize==2);
 std::printf("PASS full original factory: original pools, work area, 3x56000 sequence blocks; sequence105/chord73/ME2 archives;40ME entries/6remix groups\n");
 const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(15);
 for(const auto& entry:inventory.banks){
  auto* bank=JASTrack::sDefaultBankTable.getBank(entry.id);auto* waves=section.getWaveBankTable().getWaveBank(entry.ws);assert(bank&&waves&&bank->getWaveBank()==waves);
  for(u32 a=0;a<waves->getArcCount();++a){auto* arc=waves->getWaveArc(a);if(inventory.masks[entry.ws]&(1u<<a)){
   while(true){{JASMutexLock lock(&arc->mMutex);if(arc->mStatus==2&&arc->_5A==0)break;}assert(std::chrono::steady_clock::now()<deadline);std::this_thread::yield();}
   std::vector<u8> expected;assert(nativeReadDvdEntry(arc->mEntryNum,expected));assert(std::equal(expected.begin(),expected.end(),aramBytes.begin()+uintptr_t(arc->getHeap()->getBase())));
  }}
 }
 std::puts("STATIC LOAD begin through original AudSceneMgr policy");
 assert(system->mSceneMgr&&!system->mSceneMgr->isLoadDoneStaticResource());
 system->mSceneMgr->loadStaticResource();
 const auto staticDeadline=std::chrono::steady_clock::now()+std::chrono::seconds(20);
 while(!system->mSceneMgr->isLoadDoneStaticResource()&&std::chrono::steady_clock::now()<staticDeadline)std::this_thread::yield();
 assert(system->mSceneMgr->isLoadDoneStaticResource());
 const std::array<u32,10> staticIds{0,1,2,3,4,5,6,7,8,11};
 unsigned staticArcs=0;u64 staticBytes=0;
 for(const auto& entry:inventory.banks){
  auto* waves=section.getWaveBankTable().getWaveBank(entry.ws);
  for(u32 a=0;a<waves->getArcCount();++a){
   auto* arc=waves->getWaveArc(a);const bool required=std::find(staticIds.begin(),staticIds.end(),entry.ws)!=staticIds.end();
   JASMutexLock lock(&arc->mMutex);
   if(required){
    assert(arc->mStatus==2&&arc->_5A==0);
    std::vector<u8> expected;assert(nativeReadDvdEntry(arc->mEntryNum,expected));
    const auto at=uintptr_t(arc->getHeap()->getBase());assert(at>=0x4000&&u64(at)+expected.size()<=0xE04000);
    assert(std::equal(expected.begin(),expected.end(),aramBytes.begin()+at));staticBytes+=expected.size();++staticArcs;
   }else assert(arc->mStatus==0&&arc->_5A==0);
  }
 }
 std::printf("PASS original static policy: %u waveform archives, %llu bytes exactly match DVD; other wave sets remain unloaded\n",staticArcs,static_cast<unsigned long long>(staticBytes));
 assert(staticArcs==10);
 auto* scene=system->mSceneMgr;
 assert(scene&&scene->findSceneSoundInfo("Game","PeachCastleGardenGalaxy"));
 const auto* garden=scene->findSceneSoundInfo("Game","PeachCastleGardenGalaxy");
 assert(garden->mStageId==36&&garden->mScenarioId==-1);
 assert(scene->mBgmWaveSetId==0&&scene->mSeWaveSetId==0&&scene->mPrevPlayerMode==0);
 scene->setPlayerModeMario();
 assert(scene->isPlayerModeMario());
 scene->loadStageResource("Game","PeachCastleGardenGalaxy");
 assert(scene->mBgmWaveSetId==0&&scene->mSeWaveSetId==36&&scene->mPrevPlayerMode==1&&scene->mIsNewPlayerMode);
 const auto stageDeadline=std::chrono::steady_clock::now()+std::chrono::seconds(25);
 while(!scene->isLoadDoneStageResource()&&std::chrono::steady_clock::now()<stageDeadline)std::this_thread::yield();
 assert(scene->isLoadDoneStageResource()&&scene->isPlayerResourceLoaded());
 const std::array<u32,10> stageStaticIds{0,1,2,3,4,5,6,7,8,11};
 struct AramRange{u64 begin,end;u32 ws,arc;};
 std::vector<AramRange> loadedRanges;
 unsigned stageArcs=0,stageAddedArcs=0;u64 stageBytes=0,stageAddedBytes=0;
 bool found34=false,found37=false,found43=false;
 for(const auto& entry:inventory.banks){
  auto* waves=section.getWaveBankTable().getWaveBank(entry.ws);assert(waves&&JASTrack::sDefaultBankTable.getBank(entry.id));
  for(u32 a=0;a<waves->getArcCount();++a){
   auto* arc=waves->getWaveArc(a);const bool expected=std::find(stageStaticIds.begin(),stageStaticIds.end(),entry.ws)!=stageStaticIds.end()||entry.ws==37||entry.ws==43||(entry.ws==34&&a==1);
   JASMutexLock lock(&arc->mMutex);
   if(expected){
    assert(arc->mStatus==2&&arc->_5A==0&&arc->getHeap()&&arc->mEntryNum>=0);
    std::vector<u8> dvd;assert(nativeReadDvdEntry(arc->mEntryNum,dvd)&&!dvd.empty());
    const auto at=uintptr_t(arc->getHeap()->getBase());assert(at>=0x4000&&u64(at)+dvd.size()<=0xE04000);
    assert(std::equal(dvd.begin(),dvd.end(),aramBytes.begin()+at));
    for(const auto& prior:loadedRanges)assert(u64(at)+dvd.size()<=prior.begin||prior.end<=at);
    loadedRanges.push_back({at,u64(at)+dvd.size(),entry.ws,a});stageBytes+=dvd.size();++stageArcs;
    if(entry.ws==37||entry.ws==43||(entry.ws==34&&a==1)){stageAddedBytes+=dvd.size();++stageAddedArcs;}
    if(entry.ws==34&&a==1)found34=true;if(entry.ws==37)found37=true;if(entry.ws==43)found43=true;
   }else assert(arc->mStatus==0&&arc->_5A==0);
  }
 }
 assert(found34&&found37&&found43&&stageAddedArcs>=3&&stageArcs>=13&&stageBytes<=AudParams::aramSize);
 for(const auto& heap:system->mStreamAramMgr->mHeaps){
  const u64 begin=uintptr_t(heap.mBase),end=begin+heap.mSize;assert(begin>=0x4000&&end<=0xE04000);
  for(const auto& range:loadedRanges)assert(end<=range.begin||range.end<=begin);
 }
 std::printf("PASS original Mario Garden stage loading: stage36, SE sets37/43, BGM none, WS34 archive1; %u total archives/%llu bytes (%u newly loaded/%llu bytes) match DVD, in14MiB ARAM, disjoint from stream buffers\n",stageArcs,static_cast<unsigned long long>(stageBytes),stageAddedArcs,static_cast<unsigned long long>(stageAddedBytes));
 JAISeqDataRegion coinRegion{};assert(collection->getSeqDataRegion(&coinRegion));
 JAISeqData actualCoin(nullptr,0);assert(collection->getSeqData(JAISoundID(0),&actualCoin)==2&&actualCoin.data==coinRegion.addr&&actualCoin.offset==0x411);
 assert(nonzeroSamples.load()==0&&JAISoundStarter::getInstance()==system);
 playbackReady.store(true,std::memory_order_release);
 unsigned mainFrames=0,cycles=0,stallSubframes=0,stallDac=0,stallNonzero=0;bool delayVerified=false;
 JAISoundHandle requestedCoin;
 for(unsigned cycle=0;cycle<3;++cycle){
  const auto pcmStart=nonzeroSamples.load();const auto cycleStart=std::chrono::steady_clock::now();
  const auto cycleDeadline=cycleStart+std::chrono::seconds(15);
  assert(OSGetCurrentThread()==testMainThread);
  assert(system->startSound(JAISoundID(0),&requestedCoin,nullptr)&&requestedCoin.isSoundAttached());
  while(requestedCoin.isSoundAttached()||system->getNumActiveSounds()!=0){
   VIWaitForRetrace();assert(OSGetCurrentThread()==testMainThread);
   system->frameWork();++mainFrames;
   if(cycle==0&&mainFrames==4){
    const auto output=Native::nativeAiOutputSnapshot();
    std::printf("EARLY output readiness: backend=%u queued=%u completedFrames=%llu subframes=%u DAC=%u\n",unsigned(output.backend),output.queuedBlocks,static_cast<unsigned long long>(output.completedFrames),subframes.load(),dacChunks.load());
   }
   if(cycle==0&&!delayVerified&&nonzeroSamples.load()>pcmStart&&requestedCoin.isSoundAttached()){
    assert(requestedCoin.isSoundAttached());
    const auto beforeSubframes=subframes.load(),beforeDac=dacChunks.load(),beforePcm=nonzeroSamples.load();
    // Deliberately delay only the game-facing thread. Audio and retrace workers
    // keep their real clocks; no sound/DSP worker is paused or manually stepped.
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    stallSubframes=subframes.load()-beforeSubframes;stallDac=dacChunks.load()-beforeDac;stallNonzero=nonzeroSamples.load()-beforePcm;
    std::printf("OBSERVED delay: before subframes=%u DAC=%u nonzero=%u; delta subframes=%u DAC=%u nonzero=%u\n",beforeSubframes,beforeDac,beforePcm,stallSubframes,stallDac,stallNonzero);
    assert(stallSubframes>0&&stallDac>0&&stallNonzero>0);delayVerified=true;
    std::printf("PASS independent audio during80ms main-thread delay: subframes=%u DAC=%u nonzero=%u\n",stallSubframes,stallDac,stallNonzero);
   }
   assert(std::chrono::steady_clock::now()<cycleDeadline);
  }
  assert(nonzeroSamples.load()>pcmStart&&!system->getSeMgr().isUsingSeqData(coinRegion));
  assert(JAISe::memPool_.getFreeMemCount()==120&&AudSeStrategy::memPool_.getFreeMemCount()==120&&JAISoundChild::memPool_.getFreeMemCount()==160);
  ++cycles;
  std::printf("PASS main-thread natural coin cycle%u: actual AudSystem framework; sound/strategy/child pools restored; elapsedMs=%llu\n",cycles,static_cast<unsigned long long>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-cycleStart).count()));
 }
 std::printf("MAIN THREAD playback: cycles=%u mainFrames=%u retraces=%u subframes=%u DAC=%u nonzero=%u\n",cycles,mainFrames,VIGetRetraceCount(),subframes.load(),dacChunks.load(),nonzeroSamples.load());
 assert(cycles==3&&mainFrames>60&&subframes.load()>0&&delayVerified);
 const auto tailStart=std::chrono::steady_clock::now();const auto tailDeadline=tailStart+std::chrono::seconds(15);const auto tailFramesStart=mainFrames;
 tailDrainRequested.store(true,std::memory_order_release);
 while(!tailsRetired.load(std::memory_order_acquire)){
  VIWaitForRetrace();assert(OSGetCurrentThread()==testMainThread);system->frameWork();++mainFrames;
  assert(std::chrono::steady_clock::now()<tailDeadline);
 }
 std::printf("PASS natural release tails retire before worker stop: mainFrames=%u elapsedMs=%llu; all64DSP/72parent/200track/1extra slots return\n",mainFrames-tailFramesStart,static_cast<unsigned long long>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-tailStart).count()));
 const auto playbackStopDeadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
 audioWorker->stop();while(!OSIsThreadTerminated(audioWorker->getThreadRecord())&&std::chrono::steady_clock::now()<playbackStopDeadline)std::this_thread::yield();assert(OSIsThreadTerminated(audioWorker->getThreadRecord())&&!nativeDspAudioRuntimeActive());audioWorker->~JASAudioThread();JKRHeap::free(audioWorker,dram);
 assert(!requestedCoin.isSoundAttached()&&system->getNumActiveSounds()==0&&JASTrack::sTrackList.mTracks.size()==0);
 std::printf("POST STOP resources: DSP=%d parent=%u tracks=%u extra=%u\n",JASDSPChannel::getNumFree(),JASChannel::memPool_.getFreeMemCount(),JASTrack::memPool_.getFreeMemCount(),JASTrack::TChannelMgr::memPool_.getFreeMemCount());
 assert(JASDSPChannel::getNumFree()==64&&JASChannel::memPool_.getFreeMemCount()==72&&JASTrack::memPool_.getFreeMemCount()==200&&JASTrack::TChannelMgr::memPool_.getFreeMemCount()==1);
 assert(JASDriver::rejectCallback(playbackMonitor,nullptr)==1);
 assert(JASDriver::rejectCallback(JASTrack::TList::cbSeqMain,&JASTrack::sTrackList)==1);JASTrack::sTrackList.mCallbackRegistered=false;JASDriver::dacCallbackFunc=nullptr;
 system->getSeMgr().resetSeqDataMgr();system->getSeqMgr().resetSeqDataMgr();
 for(auto* sound:{system->mSystemSeObject,system->mAtmosphereSeObject,system->mRemixMgr->mSoundObj})sound->~AudSoundObject();
 system->mStreamAramMgr->~JAUStreamStaticAramMgr();system->mSpkSystem->~SpkSystem();SpkSpeakerCtrl::setMixingBuffer(nullptr);
 system->mRhythmMeSystem->mMeMgr->~AudMeMgr();system->mRhythmMeSystem->~AudRhythmMeSystem();
 system->~AudSystem();AudSystem::msBasic=nullptr;
 for(const auto& entry:inventory.banks){auto* waves=section.getWaveBankTable().getWaveBank(entry.ws);for(u32 a=0;a<waves->getArcCount();++a){auto* arc=waves->getWaveArc(a);if(arc->mStatus==2)assert(arc->erase());}JASTrack::sDefaultBankTable.registBank(entry.id,nullptr);section.getWaveBankTable().registWaveBank(entry.ws,nullptr);waves->~JASWaveBank();JKRHeap::free(waves,dram);}
 for(auto* archive:{seqArchive,chordArchive,meArchive}){archive->~JKRArchive();JKRHeap::free(archive,preloaded?static_cast<JKRHeap*>(dram):sectionHeap);}
 if(preloaded){suppliedRemix->~JKRArchive();JKRHeap::free(suppliedRemix,dram);}
 section.dispose();section.~JAUSectionHeap();JKRHeap::free(&section,sectionHeap);JKRHeap::destroy(sectionHeap);
 dvdWorker->~JASTaskThread();JKRHeap::free(dvdWorker,dram);
 }
 static_cast<JKRThread*>(aramWorker)->~JKRThread();JKRHeap::free(aramWorker,nullptr);
 // Coordinated test cleanup of the platform ARAM manager's graph heap.
 auto* graphHeap=manager->mAramHeap;
 while(auto* link=JKRAramHeap::sAramList.getFirst()){auto* block=link->getObject();block->~JKRAramBlock();JKRHeap::free(block,nullptr);}
 graphHeap->~JKRAramHeap();JKRHeap::free(graphHeap,nullptr);manager->mAramHeap=nullptr;
 manager->~JKRAram();JKRHeap::free(manager,platformHeap);
 assert(JKRThread::sThreadList.getNumLinks()==0&&!JKRAram::getManager());
 auto& waveRoot=JASKernel::audioAramHeap;assert(waveRoot.getFirstChild()==waveRoot.getEndChild());waveRoot.initRootHeap(nullptr,0);
 Native::nativeDspAddressSpace().clear();nativeReleaseAramTransport();nativeReleaseDvd();JKRHeap::destroy(dram);
 assert(!JASDram&&JKRFileLoader::sVolumeList.getNumLinks()==0);JKRHeap::destroy(platformHeap);assert(root.check()&&root.getTotalFreeSize()==initial);
 }
 videoScheduler.stop();videoScheduler.checkFailure();assert(VIGetRetraceCount()>0);
 assert(VirtualFree(memory,0,MEM_RELEASE));std::puts("PASS main-thread retrace-driven AudSystem framework with independent audio workers and coordinated cleanup; normal main return; not gameplay");
}
}
int main(int argc,char** argv){assert(argc==2||(argc==3&&!std::strcmp(argv[2],"--preloaded")));std::setvbuf(stdout,nullptr,_IONBF,0);auto* observer=AddVectoredExceptionHandler(1,observe);assert(observer);run(argv[1],argc==3);assert(RemoveVectoredExceptionHandler(observer));}
