$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JAUSeqCollection.cpp" -Raw
$s='#include <JSystem/JAudio2/JAUSoundTable.hpp>'+[Environment]::NewLine+$s
$s=$s.Replace('mCollectionData = nullptr;', 'mCollectionData = nullptr; mGroupNum=0; mFileSize=0; mSoundTableOffsets=nullptr;')
$s=$s.Replace('mCollectionData->magic[0]', '!mCollectionData || mCollectionData->magic[0]')
$s=$s.Replace('mGroupNum = mCollectionData->mGroupNum;', 'mGroupNum = nativeSoundTableU16(&mCollectionData->mGroupNum);')
$s=$s.Replace('mFileSize = mCollectionData->mFileSize;', 'mFileSize = nativeSoundTableU32(&mCollectionData->mFileSize);'+[Environment]::NewLine+'    if (mFileSize < 8 || mGroupNum > (mFileSize-8)/4) {mCollectionData=nullptr;return;}')
$s=$s.Replace('if (groupId >= mGroupNum)', 'if (!mCollectionData || !pSeqData || groupId < 0 || waveId < 0 || groupId >= mGroupNum)')
$s=$s.Replace('u32 offset = mSoundTableOffsets[groupId];', 'u32 offset = nativeSoundTableU32(reinterpret_cast<const u8*>(mSoundTableOffsets)+groupId*4);'+[Environment]::NewLine+'    if (offset > mFileSize || mFileSize-offset < 4) return false;')
$s=$s.Replace('if (waveId >= pSoundTable->mWaveNum)', 'const u32 count = nativeSoundTableU32(&pSoundTable->mWaveNum);'+[Environment]::NewLine+'    if (count > (mFileSize-offset-4)/4 || u32(waveId) >= count)')
$s=$s.Replace('pSeqData->set((void*)mCollectionData, pSoundTable->mWaveSeqOffsets[waveId]);', 'const u32 seqOffset = nativeSoundTableU32(reinterpret_cast<const u8*>(pSoundTable->mWaveSeqOffsets)+waveId*4);'+[Environment]::NewLine+'    if (seqOffset >= mFileSize) return false;'+[Environment]::NewLine+'    pSeqData->set((void*)mCollectionData, seqOffset);')
$s=$s.Replace('if (isValid()) {','if (pSeqDataRegion && isValid()) {')
$s=$s.Replace('getSeqDataRegion(&region);', 'if (!getSeqDataRegion(&region)) return 2;')
Set-Content "$n\src\generated\JAUSeqCollection.cpp" $s

