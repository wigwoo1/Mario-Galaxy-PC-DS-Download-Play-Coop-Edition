#define WIN32_LEAN_AND_MEAN
#include "assets.hpp"
#include "native_dvd_port.hpp"
#include <JSystem/JAudio2/JAUAudioArcInterpreter.hpp>
#include <JSystem/JAudio2/JAISeMgr.hpp>
#include <JSystem/JAudio2/JAUSoundMgr.hpp>
#include <JSystem/JAudio2/JAISeq.hpp>
#include <JSystem/JAudio2/JAUSectionHeap.hpp>
#include <JSystem/JKernel/JKRDvdArchive.hpp>
#include <bitset>
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


#include <JSystem/JAudio2/JAUAudioArcLoader.hpp>
#include <Game/AudioLib/AudSystem.hpp>
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
void run(const char* path){
 assert(nativeBindDvd(path));std::vector<u8> packed;assert(nativeReadDvdEntry(DVDConvertPathToEntrynum("/AudioRes/SMR.szs"),packed));auto reference=decodeYaz0(packed);Inventory inventory;assert(reference.size()==596320&&inventory.parse(reference.data(),reference.size())&&inventory.waveCount==48&&inventory.banks.size()==48);
 void* memory=VirtualAlloc(nullptr,0x1000000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);assert(memory);
 {
 JKRExpHeap root(memory,0x1000000,nullptr,false);JKRHeap::sRootHeap=JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();const auto initial=root.getTotalFreeSize();
 // Exercise actual handle-owner construction/destruction on reclaimable heaps.
 for(unsigned generation=0;generation<2;++generation){
  auto* handlesHeap=JKRExpHeap::create(0x10000,&root,false);assert(handlesHeap);
  const auto handlesFree=handlesHeap->getTotalFreeSize();
  for(unsigned cycle=0;cycle<8;++cycle)for(u8 count:{u8(0),u8(1),u8(8),u8(10),u8(16),u8(255)}){
   {
    JAUSoundObject object(nullptr,count,handlesHeap);
    assert(object.mNumHandles==count&&object.mIsAllocated);
    assert(count?JKRHeap::findFromRoot(object.mHandles)==handlesHeap:object.mHandles==nullptr);
    for(u32 i=0;i<count;++i)assert(!object.mHandles[i].isSoundAttached());
   }
   assert(handlesHeap->check()&&handlesHeap->getTotalFreeSize()==handlesFree);
   {
    AudMeObject object(nullptr,count,handlesHeap);
    assert(object.mNumHandles==count&&object.mIsAllocated);
    assert(count?JKRHeap::findFromRoot(object.mHandles)==handlesHeap:object.mHandles==nullptr);
    for(u32 i=0;i<count;++i)assert(!object.mHandles[i].isMeAttached());
   }
   assert(handlesHeap->check()&&handlesHeap->getTotalFreeSize()==handlesFree);
  }
  JKRHeap::destroy(handlesHeap);assert(root.check()&&root.getTotalFreeSize()==initial);
 }
 std::puts("PASS original sound-object handle lifetime: 96 SE and 96 ME cycles; counts 0/1/8/10/16/255; two real heaps fully reclaimed");
 auto* platformHeap=JKRExpHeap::create(0x200000,&root,false);assert(platformHeap);JKRHeap::sSystemHeap=platformHeap;
 auto* dram=JKRSolidHeap::create(0xC00000,&root,false);assert(dram);JASKernel::setupRootHeap(dram,0x200000);
 std::vector<u8> aramBytes(0xE00000,0xcd);auto aram=std::make_shared<NativeAramMemory>(aramBytes.data(),aramBytes.size());assert(nativeBindAramTransport(aram));
 JASHeap& waveRoot=JASKernel::audioAramHeap;waveRoot.initRootHeap(reinterpret_cast<void*>(0x4000),u32(aramBytes.size()-0x4000));auto* priorAramHeap=JASWaveArcLoader::sAramHeap;JASWaveArcLoader::sAramHeap=&waveRoot;
 JAISe::newMemPool(120);JAISeq::newMemPool(3);JAIStream::newMemPool(3);JAISoundChild::newMemPool(160);
 auto* aramWorker=JKRAramStream::create(16);assert(aramWorker&&JASDvd::createThread(16,64,0x4000));auto* dvdWorker=JASDvd::getThreadPointer();
 JASAramStream::initSystem(0x2760,4);JASTrack::newMemPool(200);JASTrack::TChannelMgr::newMemPool(1);DSPInit();Native::nativeDspAddressSpace().clear();JASAudioThread::create(16);auto* audioWorker=JASAudioThread::getThreadPointer();assert(audioWorker);
 {
 auto* priorFactoryHeap=JKRHeap::getCurrentHeap();auto* ownedSection=JAUNewSectionHeap(false);assert(ownedSection&&JKRHeap::getCurrentHeap()==priorFactoryHeap);assert(JKRHeap::getSystemHeap()==platformHeap&&platformHeap->getFreeSize()>1024);auto* sectionHeap=static_cast<JKRSolidHeap*>(ownedSection->mHeap);assert(sectionHeap&&JKRHeap::findFromRoot(ownedSection)==sectionHeap);auto& section=*ownedSection;JAUAudioArcLoader loader(&section);auto* priorHeap=JKRHeap::getCurrentHeap();
 assert(loader.load("/AudioRes/SMR.szs")&&loader.nativeArchiveSize==reference.size()&&JKRHeap::getCurrentHeap()==priorHeap);
 auto* table=section.getSoundTable();auto* names=section.sectionHeapData_.soundNameTable;auto* collection=static_cast<JAUSeqDataMgr_SeqCollection*>(section.getSeSeqDataMgr());
 assert(table&&names&&collection);std::printf("CATALOG metadata owners BST=%p BSTN=%p BSC=%p wanted=%p\n",static_cast<void*>(JKRHeap::findFromRoot(table)),static_cast<void*>(JKRHeap::findFromRoot(names)),static_cast<void*>(JKRHeap::findFromRoot(collection)),static_cast<void*>(sectionHeap));
 assert(JKRHeap::findFromRoot(table)==sectionHeap&&JKRHeap::findFromRoot(names)==sectionHeap&&JKRHeap::findFromRoot(collection)==sectionHeap);
 assert(table->nativeResourceSize==inventory.bstSize&&std::memcmp(table->getResource(),inventory.bst,inventory.bstSize)==0&&table->getResource()!=inventory.bst&&JKRHeap::findFromRoot(const_cast<void*>(table->getResource()))==sectionHeap);
 assert(std::memcmp(section.data_.mBstnDst,inventory.names,inventory.nameSize)==0&&JKRHeap::findFromRoot(const_cast<void*>(section.data_.mBstnDst))==sectionHeap);
 assert(std::memcmp(collection->getCollectionData(),inventory.bsc,inventory.bscSize)==0);
 JAISeqDataRegion region{};assert(collection->isValid()&&collection->getSeqDataRegion(&region)&&region.size==be32(inventory.bsc+4)&&region.size<=inventory.bscSize&&region.addr!=inventory.bsc&&JKRHeap::findFromRoot(region.addr)==sectionHeap&&std::memcmp(region.addr,inventory.bsc,region.size)==0);
 assert(!std::strcmp(names->getName(JAISoundID(0)),"SE_SY_COIN")&&!std::strcmp(names->getName(JAISoundID(0x01000002)),"BGM_MISS"));
 JAISeqData coin(nullptr,0);assert(collection->getSeqData(JAISoundID(0),&coin)==2&&coin.data==region.addr&&coin.offset==0x411);
 unsigned registered=0,archives=0,loaded=0;u64 loadedBytes=0;const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(15);
 for(const auto& entry:inventory.banks){
  auto* bank=JASTrack::sDefaultBankTable.getBank(entry.id);auto* waves=section.getWaveBankTable().getWaveBank(entry.ws);assert(bank&&waves&&bank->getWaveBank()==waves);
  assert(JKRHeap::findFromRoot(bank)==sectionHeap&&JKRHeap::findFromRoot(waves)==sectionHeap&&section.data_.registeredBankTables.test(entry.id)&&section.data_.registeredWaveBankTables.test(entry.ws));++registered;
  for(u32 a=0;a<waves->getArcCount();++a){++archives;auto* arc=waves->getWaveArc(a);assert(arc&&arc->mEntryNum>=0);
   if(a<32&&(inventory.masks[entry.ws]&(1u<<a))){
    while(true){{JASMutexLock lock(&arc->mMutex);if(arc->mStatus==2&&arc->_5A==0)break;}assert(std::chrono::steady_clock::now()<deadline);std::this_thread::yield();}
    std::vector<u8> bytes;assert(nativeReadDvdEntry(arc->mEntryNum,bytes));const auto base=uintptr_t(arc->getHeap()->getBase());assert(base>=0x4000&&base+bytes.size()<=aramBytes.size()&&std::equal(bytes.begin(),bytes.end(),aramBytes.begin()+base));++loaded;loadedBytes+=bytes.size();
   }else assert(arc->mStatus==0);
  }
 }
 assert(registered==48&&loaded>0);std::printf("PASS actual original catalog loader: %u banks/wave systems, %u wave archives; %u authored startup loads independently match %llu ARAM bytes; tables/collection metadata and copies owned by section\n",registered,archives,loaded,static_cast<unsigned long long>(loadedBytes));
 auto* savedCurrent=sectionHeap->becomeCurrentHeap();JKRFileLoader::initializeVolumeList();
 {
 JKRDvdArchive chordArchive(DVDConvertPathToEntrynum("/AudioRes/Info/JaiChord.arc"),JKRArchive::MOUNT_DIRECTION_1);
 JKRDvdArchive meArchive(DVDConvertPathToEntrynum("/AudioRes/Info/JaiMe.arc"),JKRArchive::MOUNT_DIRECTION_1);
 JKRDvdArchive remixArchive(DVDConvertPathToEntrynum("/AudioRes/Info/JaiRemixSeq.arc"),JKRArchive::MOUNT_DIRECTION_1);
 assert(chordArchive.mIsMounted&&meArchive.mIsMounted&&remixArchive.mIsMounted);
 std::printf("AUX archives mounted: chord=%u me=%u remix=%u; constructing actual AudSystem\n",chordArchive.countResource(),meArchive.countResource(),remixArchive.countResource());
 auto* system=AudNewAudSystem_(&section,&chordArchive,&meArchive,&remixArchive,1);assert(system&&AudSystem::get()==system);
 assert(system->mSectionHeap==&section&&system->mSoundInfo&&system->mStreamAramMgr&&system->mRhythmMeSystem&&system->mRemixMgr&&system->mSpkSystem);
 std::printf("AUDSYSTEM constructed: owner=%p section=%p infoOwner=%p streamMgrOwner=%p\n",static_cast<void*>(JKRHeap::findFromRoot(system)),static_cast<void*>(sectionHeap),static_cast<void*>(JKRHeap::findFromRoot(system->mSoundInfo)),static_cast<void*>(JKRHeap::findFromRoot(system->mStreamAramMgr)));
 assert(JKRHeap::findFromRoot(system)==sectionHeap&&JKRHeap::findFromRoot(system->mSoundInfo)==sectionHeap&&JKRHeap::findFromRoot(system->mStreamAramMgr)==sectionHeap);
 assert(system->mStreamAramMgr->mSize==2&&system->mStreamAramMgr->mBits.none());
 for(const auto& heap:system->mStreamAramMgr->mHeaps)assert(heap.mSize==36*0x2760&&uintptr_t(heap.mBase)>=0x4000+119712&&uintptr_t(heap.mBase)+heap.mSize<=aramBytes.size());
 assert(system->mRhythmMeSystem->mChordInfo.mArchive==&chordArchive&&system->mRhythmMeSystem->mMeMgr&&system->mRhythmMeSystem->mMeMgr->mMeSeq&&system->mRemixMgr->mGroupCount>0);
 assert(system->getNumActiveSounds()==0);
 std::printf("PASS actual AudSystem constructor: stream buffers=%u bytesEach=%u; ME entries=%u remix groups=%d; chord archive retained\n",system->mStreamAramMgr->mSize,system->mStreamAramMgr->mHeaps[0].mSize,system->mRhythmMeSystem->mMeMgr->mParams.mNumEntries,system->mRemixMgr->mGroupCount);
 audioWorker->stop();auto stopDeadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);while(!OSIsThreadTerminated(audioWorker->getThreadRecord())&&std::chrono::steady_clock::now()<stopDeadline)std::this_thread::yield();assert(OSIsThreadTerminated(audioWorker->getThreadRecord())&&!nativeDspAudioRuntimeActive());audioWorker->~JASAudioThread();JKRHeap::free(audioWorker,dram);audioWorker=nullptr;
 system->getSeMgr().resetSeqDataMgr();system->getSeqMgr().resetSeqDataMgr();
 // AudSoundObject inherits JKRDisposer privately, so an external dynamic_cast
 // from the disposer list cannot find it. Destroy the three owned objects
 // through their original public owner pointers while AudSystem is alive.
 for(auto* sound:{system->mSystemSeObject,system->mAtmosphereSeObject,system->mRemixMgr->mSoundObj}){
  assert(sound&&JKRHeap::findFromRoot(sound)==sectionHeap);
  std::printf("CLEANUP owned sound object %p while AudSystem and holder remain alive\n",static_cast<void*>(sound));
  sound->~AudSoundObject();
 }

 system->mStreamAramMgr->~JAUStreamStaticAramMgr();
 system->mSpkSystem->~SpkSystem();SpkSpeakerCtrl::setMixingBuffer(nullptr);
 system->mRhythmMeSystem->mMeMgr->~AudMeMgr();system->mRhythmMeSystem->~AudRhythmMeSystem();

 system->~AudSystem();AudSystem::msBasic=nullptr;
 }
 savedCurrent->becomeCurrentHeap();
 auto* input=const_cast<u8*>(loader.mBase);assert(input&&JKRHeap::findFromRoot(input)==sectionHeap);
 sectionHeap->freeTail();auto* overwrite=static_cast<u8*>(sectionHeap->alloc(u32(reference.size()),-32));assert(overwrite==input);std::memset(overwrite,0xa5,reference.size());
 assert(std::memcmp(table->getResource(),inventory.bst,inventory.bstSize)==0&&std::memcmp(section.data_.mBstnDst,inventory.names,inventory.nameSize)==0&&std::memcmp(region.addr,inventory.bsc,region.size)==0);
 assert(!std::strcmp(names->getName(JAISoundID(0)),"SE_SY_COIN")&&collection->getSeqData(JAISoundID(0),&coin)==2&&coin.offset==0x411);
 JASInstParam sample;assert(JASTrack::sDefaultBankTable.getBank(0)->getInstParam(0,60,100,&sample)&&std::isfinite(sample.mPitch)&&std::isfinite(sample.mVolume));
 std::puts("PASS original temporary BAA tail reclaimed and same span overwritten; copied BST/BSTN/BSC lookups and instrument parameters survive");
 for(const auto& entry:inventory.banks){auto* waves=section.getWaveBankTable().getWaveBank(entry.ws);for(u32 a=0;a<waves->getArcCount();++a){auto* arc=waves->getWaveArc(a);if(arc->mStatus==2)assert(arc->erase());}JASTrack::sDefaultBankTable.registBank(entry.id,nullptr);section.getWaveBankTable().registWaveBank(entry.ws,nullptr);waves->~JASWaveBank();JKRHeap::free(waves,dram);}
 section.dispose();assert(!JAUSoundTable::getInstance()&&!JAUSoundNameTable::getInstance());std::puts("PASS original section disposal clears catalog singletons and all registered bank/ARAM references removed");
 ownedSection->~JAUSectionHeap();JKRHeap::free(ownedSection,sectionHeap);JKRHeap::destroy(sectionHeap);
 }
 assert(!audioWorker);
 dvdWorker->~JASTaskThread();JKRHeap::free(dvdWorker,dram);static_cast<JKRThread*>(aramWorker)->~JKRThread();JKRHeap::free(aramWorker,nullptr);assert(!JASDvd::getThreadPointer()&&JKRThread::sThreadList.getNumLinks()==0);
 assert(waveRoot.getFirstChild()==waveRoot.getEndChild());waveRoot.initRootHeap(nullptr,0);JASWaveArcLoader::sAramHeap=priorAramHeap;Native::nativeDspAddressSpace().clear();nativeReleaseAramTransport();nativeReleaseDvd();JKRHeap::destroy(dram);assert(!JASDram&&JAISe::memPool_.getTotalMemCount()==0&&JAISeq::memPool_.getTotalMemCount()==0&&JAIStream::memPool_.getTotalMemCount()==0&&JAISoundChild::memPool_.getTotalMemCount()==0);JKRHeap::destroy(platformHeap);assert(root.check()&&root.getTotalFreeSize()==initial);
 }
 assert(VirtualFree(memory,0,MEM_RELEASE));std::puts("PASS complete catalog initialization component, worker/heap/backing cleanup and normal main return; not gameplay");
}
}
int main(int argc,char** argv){assert(argc==2);std::setvbuf(stdout,nullptr,_IONBF,0);auto* observer=AddVectoredExceptionHandler(1,observe);assert(observer);run(argv[1]);assert(RemoveVectoredExceptionHandler(observer));}
