$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out="$n\build\camera-pointer-candidate\include\Game\Speaker"
New-Item -ItemType Directory -Path $out -Force|Out-Null
$h=Get-Content "$p\Petari\include\Game\Speaker\SpkData.hpp" -Raw
$h="#include <Native/OwnedSpeakerTable.hpp>`n#include <Native/OwnedSpeakerWave.hpp>`n"+$h
$h=$h.Replace('    /* 0x20 */ JKRArchive* mArchive;', '    /* 0x20 */ JKRArchive* mArchive;'+[Environment]::NewLine+'    Native::OwnedSpeakerTable mNativeTable;'+[Environment]::NewLine+'    Native::OwnedSpeakerWave mNativeWave;')
Set-Content "$out\SpkData.hpp" $h
$src=@"
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
"@
Set-Content "$n\src\generated\SpkData.cpp" $src
$s=Get-Content "$p\Petari\src\Game\Speaker\SpkTable.cpp" -Raw
$start=$s.IndexOf('SpkTable::SpkTable()');$brace=$s.IndexOf('{',$start);$end=$s.IndexOf('}',$brace)+1
Set-Content "$n\src\generated\SpkTableConstructor.cpp" ('#include <Game/Speaker/SpkTable.hpp>'+[Environment]::NewLine+$s.Substring($start,$end-$start))
