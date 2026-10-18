#include <Game/System/GameSystemStationedArchiveLoader.hpp>
#include <Game/System/StationedFileInfo.hpp>
#include <JSystem/JKernel/JKRExpHeap.hpp>
#include <cassert>
#include <cstdio>
#include <type_traits>
int main(){
 static_assert(std::is_abstract<StationedArchiveLoader::Condition>::value,"Base condition must not supply invented policy");
 ConditionIfIsNotPlayer others;ConditionIsEqualType mario(true),luigi(false);ConditionUsePlayerHeap player;
 assert(player.mIsDataMario&&player.mNapaHeap==nullptr&&player.mGDDRHeap==nullptr);
 player.mNapaHeap=reinterpret_cast<JKRExpHeap*>(uintptr_t(0x10000));
 player.mGDDRHeap=reinterpret_cast<JKRExpHeap*>(uintptr_t(0x20000));
 unsigned count=0,marioCount=0,luigiCount=0,otherCount=0;
 for(auto entry=MR::getStationedFileInfoTable();entry->mArchive;++entry){
  bool isMario=entry->mLoadType==MR::StationedFileInfo::LOAD_TYPE_MOUNT_RESOURCE_MARIO;
  bool isLuigi=entry->mLoadType==MR::StationedFileInfo::LOAD_TYPE_MOUNT_RESOURCE_LUIGI;
  assert(mario.isExecute(entry)==isMario&&luigi.isExecute(entry)==isLuigi);
  assert(others.isExecute(entry)==(!isMario&&!isLuigi));
  player.mIsDataMario=true;assert(player.isExecute(entry)==isMario);
  player.mIsDataMario=false;assert(player.isExecute(entry)==isLuigi);
  JKRHeap* expected=entry->mHeapType==MR::StationedFileInfo::HEAP_TYPE_NAPA?static_cast<JKRHeap*>(player.mNapaHeap):
    entry->mHeapType==MR::StationedFileInfo::HEAP_TYPE_GDDR?static_cast<JKRHeap*>(player.mGDDRHeap):nullptr;
  assert(player.getProperHeap(entry)==expected);
  assert(others.getProperHeap(entry)==nullptr);
  ++count;marioCount+=isMario;luigiCount+=isLuigi;otherCount+=!isMario&&!isLuigi;
 }
 assert(count>50&&marioCount>0&&luigiCount>0&&otherCount>0);
 std::printf("PASS original stationed archive policies: %u resources (%u Mario, %u Luigi, %u others), player heap routing; abstract base\n",count,marioCount,luigiCount,otherCount);
}
