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


namespace {
std::mutex mutex;std::condition_variable changed;
JASTrack* live=nullptr;OSThread* mainThread=nullptr;
unsigned subframes=0,dacFrames=0,nonzero=0,active=0,voiceChecks=0,dspWaveChecks=0;bool finished=false;
std::array<JASWaveBank*,2> expectedWaveBanks{};std::array<u32,2> archiveBases{},archiveLengths{},bankVoiceChecks{};JAISeqMgr* manager=nullptr;JAISoundHandle* soundHandle=nullptr;
unsigned cycles=0,cycleNonzero=0;std::bitset<256> programs;JAUSoundMgr* router=nullptr;JAUSectionHeap* section=nullptr;const std::vector<u8>* expectedSequence=nullptr;unsigned sequenceChecks=0,regionChecks=0;JAISeqDataRegion expectedRegion{};u32 archiveBase=0,archiveLength=0;
void inspect(JASTrack* track){
 if(!track)return;
 for(u32 m=0;m<track->mChannelMgrCount;++m)if(auto* mgr=track->mChannelMgrs[m])for(auto* ch:mgr->mChannels)if(ch){
  ++active;assert(ch->mChannelType==JASChannel::CH_WAVE&&ch->mBankDisposeID);
  auto* bank=static_cast<const JASBank*>(ch->mBankDisposeID);assert(track->mBankNumber<2&&bank->getWaveBank()==expectedWaveBanks[track->mBankNumber]);++bankVoiceChecks[track->mBankNumber];
  assert(track->mBankNumber<2&&track->mProgNumber<240);programs.set(track->mProgNumber);
  const u32 at=u32(ch->mWavePtr),bi=track->mBankNumber;assert(at>=archiveBases[bi]&&u64(at)+ch->mWaveInfo.mAWLength<=u64(archiveBases[bi])+archiveLengths[bi]);++voiceChecks;
  auto* dsp=ch->mDspCh?ch->mDspCh->mChannel:nullptr;
  if(dsp&&dsp->isActive()){assert(u32(dsp->_118)==u32(ch->mWavePtr)&&dsp->_11C==ch->mWaveInfo.mSampleCount&&dsp->_102==ch->mWaveInfo.mLoopFlags);++dspWaveChecks;}
 }
 for(auto* child:track->mChildren)if(child)inspect(child);
}
s32 monitor(void*){
 assert(OSGetCurrentThread()!=mainThread);std::lock_guard<std::mutex> lock(mutex);++subframes;
 if(subframes%7==0){router->calc();router->mixOut();}
 assert(router->getSeMgr().getNumActiveSe()==0&&router->getStreamMgr().getNumActiveStreams()==0);
 live=soundHandle->isSoundAttached()?soundHandle->getSound()->getTrack():nullptr;inspect(live);
 if(live&&live->mSeqCtrl.mReader.mSeqBuff){
  bool located=false;
  for(auto* link=section->sectionHeapData_.seqDataBlocks.getLoadedBlocks().getFirst();link;link=link->getNext())if(link->getObject()->region.addr==live->mSeqCtrl.mReader.mSeqBuff){expectedRegion=link->getObject()->region;located=true;assert(JKRHeap::findFromRoot(link->getObject())==section->mHeap);}
  assert(located);
  assert(live->mSeqCtrl.mReader.mSeqBuff==expectedRegion.addr&&manager->isUsingSeqData(expectedRegion));++regionChecks;
  assert(section->releaseIdleDynamicSeqDataBlock()==0); // Active sequence retains its borrowed bytes.
  assert(std::equal(expectedSequence->begin(),expectedSequence->end(),expectedRegion.addr));++sequenceChecks;
 }
 const bool drained=!soundHandle->isSoundAttached()&&manager->getNumActiveSeqs()==0&&JASTrack::sTrackList.mTracks.size()==0&&JASDSPChannel::getNumFree()==64&&JASChannel::memPool_.getFreeMemCount()==72;
 if(drained&&cycles<3){
  assert(JAISeq::memPool_.getFreeMemCount()==3&&JAISe::memPool_.getFreeMemCount()==120&&JASTrack::memPool_.getFreeMemCount()==200&&JASTrack::TChannelMgr::memPool_.getFreeMemCount()==1&&JAISoundChild::memPool_.getFreeMemCount()==160);
  assert(!manager->isUsingSeqData(expectedRegion)&&nonzero>cycleNonzero&&sequenceChecks>0);
  assert(section->sectionHeapData_.seqDataBlocks.getLoadedBlocks().getNumLinks()==1);
  assert(section->releaseIdleDynamicSeqDataBlock()==expectedRegion.size);
  assert(section->sectionHeapData_.seqDataBlocks.getLoadedBlocks().getNumLinks()==0&&section->sectionHeapData_.seqDataBlocks.getFreeBlocks().getNumLinks()==3);
  ++cycles;std::printf("PASS natural BGM_MISS cycle%u: handle detached, borrowed sequence released, archive block reusable and original sequence/track/channel pools restored\n",cycles);
  if(cycles<3){cycleNonzero=nonzero;assert(JAISoundStarter::getInstance()->startSound(JAISoundID(0x01000002),soundHandle,nullptr));assert(soundHandle->isSoundAttached());}
  else finished=true;
 }
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
 std::printf("MUSIC track=%p depth=%u bank=%u program=%u bankObject=%p status=%u seqOffset=%u\n",static_cast<const void*>(t),depth,unsigned(t->mBankNumber),unsigned(t->mProgNumber),t->mBankTable?static_cast<void*>(t->mBankTable->getBank(t->mBankNumber)):nullptr,unsigned(t->mStatus),t->mSeqCtrl.mReader.mSeqCursor?u32(t->mSeqCtrl.mReader.mSeqCursor-t->mSeqCtrl.mReader.mSeqBuff):0);
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
 const u8* wave=nullptr;const u8* bank=nullptr;const u8* bankZero=nullptr;const u8* waveZero=nullptr;std::array<const u8*,256> waves{};const u8* bst=nullptr;const u8* bstn=nullptr;const u8* bsc=nullptr;u32 bstBytes=0,bscBytes=0;u32 waveIndex=0;unsigned waveCount=0,bankCount=0;
 void readWS(u32 id,const void* p,u32)override{assert(id<waves.size());++waveCount;waves[id]=static_cast<const u8*>(p);}
 void readBNK(u32 ws,const void* p)override{++bankCount;if(JASBNKParser::getBankNumber(p)==0){assert(!bankZero&&ws==0);bankZero=static_cast<const u8*>(p);waveZero=waves[ws];}if(JASBNKParser::getBankNumber(p)==1){assert(!bank&&ws<waves.size());bank=static_cast<const u8*>(p);waveIndex=ws;wave=waves[ws];}}
 void readBSC(const void* p,u32 n)override{bsc=static_cast<const u8*>(p);bscBytes=n;}void readBST(const void* p,u32 n)override{bst=static_cast<const u8*>(p);bstBytes=n;}void readBSTN(const void* p,u32)override{bstn=static_cast<const u8*>(p);}void readBMS(u32,const void*,u32)override{}void readBMS_fromArchive(u32)override{}void newVoiceBank(u32,u32)override{}void newDynamicSeqBlock(u32)override{}void readBSFT(const void*)override{}void readMaxSeCategory(int,int,int)override{}void beginBNKList(u32,u32)override{}void endBNKList()override{}bool readCommandMore(u32)override{assert(false);return false;}
};
void noteWorker(const char* wbfs){
 mainThread=OSGetCurrentThread();assert(nativeBindDvd(wbfs));
 std::vector<u8> packed;const int smr=DVDConvertPathToEntrynum("/AudioRes/SMR.szs");assert(smr>=0&&nativeReadDvdEntry(smr,packed));auto bytes=decodeYaz0(packed);
 Catalog catalog;assert(bytes.size()==596320&&catalog.parse(bytes.data(),bytes.size())&&catalog.wave&&catalog.bank&&catalog.waveCount==48&&catalog.bankCount==48);
 const u32 bankNumber=JASBNKParser::getBankNumber(catalog.bank);assert(bankNumber==1&&catalog.waveIndex==1);
 std::printf("Retail catalog: %u banks/%u waveform systems; selected bank=%u waveSystem=%u\n",catalog.bankCount,catalog.waveCount,bankNumber,catalog.waveIndex);
 void* memory=VirtualAlloc(nullptr,0x800000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);assert(memory);
 {
 JKRExpHeap root(memory,0x800000,nullptr,false);JKRHeap::sRootHeap=JKRHeap::sSystemHeap=&root;root.becomeCurrentHeap();
 const auto freeBefore=root.getTotalFreeSize();auto* dram=JKRSolidHeap::create(0x500000,&root,false);assert(dram);JASKernel::setupRootHeap(dram,0x200000);
 auto* bankHeap=JKRExpHeap::create(0x100000,&root,false);assert(bankHeap);
 auto* bank=JASBNKParser::Ver1::createBasicBank(catalog.bank,bankHeap);assert(bank);
 auto* waveBank=JASWSParser::createWaveBank(catalog.wave,bankHeap);assert(waveBank&&waveBank->getArcCount()==1);bank->assignWaveBank(waveBank);
 assert(catalog.bankZero&&catalog.waveZero);auto* bankZero=JASBNKParser::Ver1::createBasicBank(catalog.bankZero,bankHeap);auto* waveZero=JASWSParser::createWaveBank(catalog.waveZero,bankHeap);assert(bankZero&&waveZero&&waveZero->getArcCount()==1);bankZero->assignWaveBank(waveZero);auto* arcZero=waveZero->getWaveArc(0);assert(arcZero&&arcZero->mEntryNum==DVDConvertPathToEntrynum("/AudioRes/Waves/B00mahit_0.aw"));
 auto* arc=waveBank->getWaveArc(0);assert(arc&&arc->mEntryNum==DVDConvertPathToEntrynum("/AudioRes/Waves/B01mahit_0.aw"));
 std::vector<u8> expected;assert(nativeReadDvdEntry(arc->mEntryNum,expected));
 std::vector<u8> zeroBytes;assert(nativeReadDvdEntry(arcZero->mEntryNum,zeroBytes));
 std::vector<u8> aramBytes(((expected.size()+31)&~size_t(31))+((zeroBytes.size()+31)&~size_t(31))+0x4000,0xcd);auto aram=std::make_shared<NativeAramMemory>(aramBytes.data(),aramBytes.size());assert(nativeBindAramTransport(aram));
 JASHeap waveRoot;waveRoot.initRootHeap(reinterpret_cast<void*>(uintptr_t(0x4000)),u32(aramBytes.size()-0x4000));
 auto* streamWorker=JKRAramStream::create(16);assert(streamWorker);assert(JASDvd::createThread(16,64,0x4000));auto* dvdWorker=JASDvd::getThreadPointer();assert(dvdWorker);
 assert(arc->load(&waveRoot));
 auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(10);
 while(true){{JASMutexLock lock(&arc->mMutex);if(arc->mStatus==2&&arc->_5A==0)break;}assert(std::chrono::steady_clock::now()<deadline);std::this_thread::yield();}
 const auto waveBase=uintptr_t(arc->getHeap()->getBase());assert(waveBase>=0x4000&&waveBase+expected.size()<=aramBytes.size());assert(std::equal(expected.begin(),expected.end(),aramBytes.begin()+waveBase));archiveBase=u32(waveBase);archiveLength=u32(expected.size());
 std::printf("PASS bank1 waveform-system1 archive B01mahit_0.aw exact transfer: %u bytes at%x\n",archiveLength,archiveBase);
 expectedWaveBanks[1]=waveBank;archiveBases[1]=archiveBase;archiveLengths[1]=archiveLength;JASTrack::sDefaultBankTable.registBank(bankNumber,bank);
 assert(arcZero->load(&waveRoot));
 deadline=std::chrono::steady_clock::now()+std::chrono::seconds(10);
 while(true){{JASMutexLock lock(&arcZero->mMutex);if(arcZero->mStatus==2&&arcZero->_5A==0)break;}assert(std::chrono::steady_clock::now()<deadline);std::this_thread::yield();}
 const auto zeroBase=uintptr_t(arcZero->getHeap()->getBase());assert(zeroBase>=0x4000&&zeroBase+zeroBytes.size()<=aramBytes.size()&&std::equal(zeroBytes.begin(),zeroBytes.end(),aramBytes.begin()+zeroBase));
 assert(zeroBase>=u64(archiveBases[1])+archiveLengths[1]||u64(zeroBase)+zeroBytes.size()<=archiveBases[1]);
 expectedWaveBanks[0]=waveZero;archiveBases[0]=u32(zeroBase);archiveLengths[0]=u32(zeroBytes.size());JASTrack::sDefaultBankTable.registBank(0,bankZero);
 std::printf("PASS bank0 waveform-system0 archive B00mahit_0.aw exact transfer: %zu bytes at%x; actual BGM_MISS needs both bank0 and bank1\n",zeroBytes.size(),u32(zeroBase));
 JAUSoundTable table(true);table.init(catalog.bst);table.nativeResourceSize=catalog.bstBytes;JAUSoundNameTable names(true);names.init(catalog.bstn);JAUStdSoundInfo info(true);
 std::vector<u8> archiveBytes;const int archiveEntry=DVDConvertPathToEntrynum("/AudioRes/Seqs/JaiSeq.arc");assert(archiveEntry>=0&&nativeReadDvdEntry(archiveEntry,archiveBytes));
 Rarc referenceArchive(archiveBytes);auto compressed=referenceArchive.readFile("/smg_jingle01.szs");auto sequenceBytes=decodeYaz0(compressed);assert(sequenceBytes.size()==864);expectedSequence=&sequenceBytes;
 JKRFileLoader::initializeVolumeList();
 {
 JKRDvdArchive sequenceArchive(archiveEntry,JKRArchive::MOUNT_DIRECTION_1);assert(sequenceArchive.mIsMounted&&sequenceArchive.countResource()==105);
 auto* archiveRecord=sequenceArchive.findIdResource(80);assert(archiveRecord&&archiveRecord->mDataSize==compressed.size());
 JAUSectionHeap sequenceSection(dram,false,0);section=&sequenceSection;sequenceSection.setSeqDataArchive(&sequenceArchive);
 const auto sectionFreeBefore=dram->getFreeSize();auto* priorHeap=JKRHeap::getCurrentHeap();
 assert(sequenceSection.newDynamicSeqBlocks(0xDAC0,3)==3&&JKRHeap::getCurrentHeap()==priorHeap);
 assert(sequenceSection.sectionHeapData_.seqDataBlocks.getFreeBlocks().getNumLinks()==3);
 unsigned ownedBlocks=0;
 for(auto* link=sequenceSection.sectionHeapData_.seqDataBlocks.getFreeBlocks().getFirst();link;link=link->getNext()){
  auto* item=link->getObject();const bool metadataOwned=JKRHeap::findFromRoot(item)==dram,payloadOwned=JKRHeap::findFromRoot(item->region.addr)==dram;
  std::printf("SECTION block%u metadataHeap=%p wanted=%p payloadHeap=%p bytes=%u\n",ownedBlocks,static_cast<void*>(JKRHeap::findFromRoot(item)),static_cast<void*>(dram),static_cast<void*>(JKRHeap::findFromRoot(item->region.addr)),item->region.size);
  assert(metadataOwned&&payloadOwned&&item->region.size==0xDAC0&&item->mSoundID.isAnonymous()&&uintptr_t(item->region.addr)%32==0);++ownedBlocks;
 }
 assert(ownedBlocks==3&&sectionFreeBefore-dram->getFreeSize()>=3*(0xDAC0+sizeof(JAUSeqDataBlock)+sizeof(JSULink<JAUSeqDataBlock>)));
 std::printf("PASS original newDynamicSeqBlocks creates3 heap-owned metadata/payload blocks; section consumption=%u bytes; current heap restored\n",sectionFreeBefore-dram->getFreeSize());
 assert(!std::strcmp(names.getName(JAISoundID(0x01000002)),"BGM_MISS"));assert(info.getBgmSeqResourceID(JAISoundID(0x01000002))==80);
 std::printf("PASS actual BGM_MISS ID01000002 catalog -> archive resource80 smg_jingle01.szs: %zu compressed/%zu decoded bytes; original DVD-mode archive/section/dynamic-block loader\n",compressed.size(),sequenceBytes.size());
 JAISeq::newMemPool(3);JAIStream::newMemPool(3);JAISe::newMemPool(120);JASTrack::newMemPool(200);JASTrack::TChannelMgr::newMemPool(1);JAISoundChild::newMemPool(160);
 DSPInit();Native::nativeDspAddressSpace().clear();JASDsp::setDSPMixerLevel(.25f);
 {
 JAUSoundMgr system;router=&system;auto& sequenceManager=system.getSeqMgr();manager=&sequenceManager;sequenceManager.setSeqDataMgr(&sequenceSection);JAISoundHandle handle;soundHandle=&handle;
 assert(JAISoundStarter::getInstance()->startSound(JAISoundID(0x01000002),&handle,nullptr));assert(handle.isSoundAttached());
 const u8* bst=catalog.bst;const u32 bstRoot=be32(bst+12),sectionOffset=be32(bst+bstRoot+8),group=be32(bst+sectionOffset+4),descriptor=be32(bst+group+16),record=descriptor&0xffffff;
 extent(record,8,catalog.bstBytes);assert(descriptor==0x600083a2&&bst[record]==112&&bst[record+1]==115&&((u16(bst[record+2])<<8)|bst[record+3])==80);
 assert(handle.getSound()->asSeq()&&!handle.getSound()->asSe());assert(info.getSoundType(JAISoundID(0x01000002))==1&&info.getPriority(JAISoundID(0x01000002))==112);
 assert(std::fabs(handle.getSound()->mParams.mProperty.mVolume-115.0f/255.0f)<.000001f);
 std::printf("PASS actual BGM descriptor=%08x priority112 authored volume115/255 resource80\n",descriptor);
 assert(JASDriver::registerDspSyncCallback(monitor,nullptr));JASDriver::dacCallbackFunc=pcm;
 JASAudioThread::create(16);auto* audio=JASAudioThread::getThreadPointer();assert(audio);
 {
 std::unique_lock<std::mutex> lock(mutex);
 bool pass=changed.wait_for(lock,std::chrono::seconds(30),[]{return finished&&active>0&&nonzero>0&&dacFrames>=30;});
 std::printf("actual music sequence: subframes=%u active=%u dac=%u nonzero=%u finish=%u trackStatus=%u parentsFree=%u dspFree=%d\n",subframes,active,dacFrames,nonzero,unsigned(finished),live?unsigned(live->mStatus):99,JASChannel::memPool_.getFreeMemCount(),JASDSPChannel::getNumFree());assert(pass&&voiceChecks>0&&dspWaveChecks>0&&programs.count()>0&&sequenceChecks>0&&regionChecks>0&&bankVoiceChecks[0]>0&&bankVoiceChecks[1]>0);
 std::printf("PASS actual music voice identity/metadata checks=%u; actual DSP source/count/loop checks=%u; bank0=%u bank1=%u sequenceByteChecks=%u borrowedRegionChecks=%u\n",voiceChecks,dspWaveChecks,bankVoiceChecks[0],bankVoiceChecks[1],sequenceChecks,regionChecks);
 }
 audio->stop();deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);
 while(!OSIsThreadTerminated(audio->getThreadRecord())&&std::chrono::steady_clock::now()<deadline)std::this_thread::yield();
 assert(OSIsThreadTerminated(audio->getThreadRecord())&&!nativeDspAudioRuntimeActive());
 assert(JASDriver::rejectCallback(monitor,nullptr)==1);assert(JASDriver::rejectCallback(JASTrack::TList::cbSeqMain,&JASTrack::sTrackList)==1);JASTrack::sTrackList.mCallbackRegistered=false;
 sequenceManager.resetSeqDataMgr();
 JASDriver::dacCallbackFunc=nullptr;AIRegisterDMACallback(nullptr);audio->~JASAudioThread();JKRHeap::free(audio,dram);
 }
 assert(sequenceSection.sectionHeapData_.seqDataUser==nullptr);
 for(auto* link=sequenceSection.sectionHeapData_.seqDataBlocks.getFreeBlocks().getFirst();link;link=link->getNext())assert(link->getObject()->mSoundID.isAnonymous());
 assert(sequenceSection.newStaticSeqData(JAISoundID(0x01000002),sequenceBytes.data(),u32(sequenceBytes.size())));
 auto* staticLink=sequenceSection.data_.field_0x28.getFirst();assert(staticLink&&staticLink->getNext()==nullptr);auto* staticBlock=staticLink->getObject();
 assert(JKRHeap::findFromRoot(staticLink)==dram&&JKRHeap::findFromRoot(staticBlock)==dram&&JKRHeap::findFromRoot(staticBlock->region.addr)==dram);
 JAISeqData staticData(nullptr,0);assert(sequenceSection.getSeqData(JAISoundID(0x01000002),&staticData)==2&&staticData.data==staticBlock->region.addr&&staticData.offset==0);
 assert(std::equal(sequenceBytes.begin(),sequenceBytes.end(),staticBlock->region.addr));
 std::printf("PASS original static sequence block, ownership-list link and deep-copied%zu bytes owned by section heap; original lookup returns that copy\n",sequenceBytes.size());
 } // Archive, section and dynamic block outlive requests/worker transfer.
 JASTrack::sDefaultBankTable.registBank(bankNumber,nullptr);JASTrack::sDefaultBankTable.registBank(0,nullptr);assert(arc->erase()&&arcZero->erase());
 dvdWorker->~JASTaskThread();JKRHeap::free(dvdWorker,dram);
 static_cast<JKRThread*>(streamWorker)->~JKRThread();JKRHeap::free(streamWorker,nullptr);
 assert(!JASAudioThread::getThreadPointer()&&!JASDvd::getThreadPointer()&&JKRThread::sThreadList.getNumLinks()==0);
 static_cast<JASSimpleWaveBank*>(waveZero)->~JASSimpleWaveBank();JKRHeap::free(waveZero,bankHeap);
 static_cast<JASSimpleWaveBank*>(waveBank)->~JASSimpleWaveBank();JKRHeap::free(waveBank,bankHeap);
 JKRHeap::destroy(bankHeap);Native::nativeDspAddressSpace().clear();nativeReleaseAramTransport();JKRHeap::destroy(dram);
 assert(!JASDram&&root.check()&&root.getTotalFreeSize()==freeBefore);
 }
 assert(VirtualFree(memory,0,MEM_RELEASE));nativeReleaseDvd();
 std::puts("PASS actual BGM_MISS catalog/archive/JAUSectionHeap dynamic loader/JAISeq request -> unchanged retail sequence -> game track/bank/channel/audio/DSP/native AI/DAC; three natural completions and heap cleanup");
}
}
int main(int argc,char** argv){assert(argc==2);std::setvbuf(stdout,nullptr,_IONBF,0);auto* observer=AddVectoredExceptionHandler(1,observe);assert(observer);noteWorker(argv[1]);assert(RemoveVectoredExceptionHandler(observer));}
