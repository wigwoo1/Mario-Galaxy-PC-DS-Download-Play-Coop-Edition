$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\AudioLib\AudSoundNameConverter.cpp" -Raw
$s=$s.Replace('return getSoundID(pName, JGadget::getHashCode(pName));','if (!pName) return -1;'+[Environment]::NewLine+'    return getSoundID(pName, JGadget::getHashCode(pName));')
$s=$s.Replace('bool isSE;','if (!pName || !mGroupItemOffsets || !mSoundNameData || std::strlen(pName)<2) return -1;'+[Environment]::NewLine+'    bool isSE;')
$s=$s.Replace('startingOffset = mGroupItemOffsets[getSeSoundCategory(pName)];','if (std::strlen(pName)<5) return -1;'+[Environment]::NewLine+'        const s32 category=getSeSoundCategory(pName);'+[Environment]::NewLine+'        '+[Environment]::NewLine+'        startingOffset = category<0 ? 0 : mGroupItemOffsets[category];').Replace('startingOffset = mGroupItemOffsets[getOtherSoundCategory(pName)];','const s32 category=getOtherSoundCategory(pName);'+[Environment]::NewLine+'        '+[Environment]::NewLine+'        startingOffset = category<0 ? 0 : mGroupItemOffsets[category];')
$s=$s.Replace('initDataTable(table);','if (!table || !table->mTable.mData) return;'+[Environment]::NewLine+'    initDataTable(table);').Replace('new u32[17];','new u32[17]();')
Set-Content "$n\src\generated\SoundNameConverterKernel.cpp" $s

$s=Get-Content "$n\src\generated\SoundNameConverterKernel.cpp" -Raw
$s=$s.Replace('for (s32 i = startingOffset; i < mNumItems; i++)','for (s32 i = 0; i < mNumItems; i++)')
Set-Content "$n\src\generated\SoundNameConverterKernel.cpp" $s
