#include <Game/Speaker/SpkData.hpp>
SpkData::SpkData(JKRArchive* archive):mArchive(archive){}
void SpkData::loadTable(u16 id){
 if(!mArchive)return;void* bytes=mArchive->getResource(id);
 if(!bytes)return;s32 size=mArchive->getResSize(bytes);
 if(size<0||!mNativeTable.load(bytes,static_cast<size_t>(size)))return;
 mNativeTable.bind(mTable);
}
void SpkData::loadWave(u16 id){
 if(!mArchive)return;void* bytes=mArchive->getResource(id);
 if(!bytes)return;s32 size=mArchive->getResSize(bytes);
 if(size<0||!mNativeWave.load(bytes,static_cast<size_t>(size)))return;
 mNativeWave.bind(mWave);
}
bool SpkData::isValid() const{
 if(!mTable.mInitialized||!mWave.mResource)return false;
 for(s32 i=0;i<mTable.mResourceCount;++i)
  if(mTable.mParameters[i].mWaveID>=mNativeWave.count())return false;
 return true;
}
