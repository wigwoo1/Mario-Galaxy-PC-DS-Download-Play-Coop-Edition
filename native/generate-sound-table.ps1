$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$h=Get-Content "$p\Petari\libs\JSystem\include\JSystem\JAudio2\JAUSoundTable.hpp" -Raw
$helper=@"
inline u16 nativeSoundTableU16(const void* ptr) { const auto* b=static_cast<const u8*>(ptr); return (u16(b[0])<<8)|b[1]; }
inline u32 nativeSoundTableU32(const void* ptr) { const auto* b=static_cast<const u8*>(ptr); return (u32(b[0])<<24)|(u32(b[1])<<16)|(u32(b[2])<<8)|b[3]; }
"@
$h=$h.Replace('struct JAUSoundTableItem {',$helper+[Environment]::NewLine+'struct JAUSoundTableItem {')
$h=$h.Replace('mData = pData;','reset();'+[Environment]::NewLine+'        if (!pData) return;'+[Environment]::NewLine+'        mData = pData;').Replace('*(u32*)mData','nativeSoundTableU32(mData)').Replace('*((u32*)mData + 3)','nativeSoundTableU32(static_cast<const u8*>(mData)+12)')
$h=$h.Replace('if (index < 0) {'+'', 'if (index < 0) {')
$h=$h.Replace('(u32)index >= mRoot->mSectionNumber','!mRoot || (u32)index >= nativeSoundTableU32(&mRoot->mSectionNumber)').Replace('u32 offset = mRoot->mSectionOffsets[index];','u32 offset = nativeSoundTableU32(reinterpret_cast<const u8*>(mRoot)+4+index*4);')
$h=$h.Replace('(u32)index >= section->mNumGroups','!mData || !section || (u32)index >= nativeSoundTableU32(&section->mNumGroups)').Replace('u32 offset = group->getItemOffset(index);','if (!mData || !group || index < 0) return nullptr;'+[Environment]::NewLine+'        u32 offset = group->getItemOffset(index);')
$h=$h.Replace('index >= mNumGroups','u32(index) >= nativeSoundTableU32(&mNumGroups)').Replace('index >= mNumItems','u32(index) >= nativeSoundTableU32(&mNumItems)')
$h=$h.Replace('return mGroupOffsets[index];','return nativeSoundTableU32(reinterpret_cast<const u8*>(mGroupOffsets)+index*4);').Replace('return mItemOffsets[index] & 0xffffff;','return nativeSoundTableU32(reinterpret_cast<const u8*>(mItemOffsets)+index*4) & 0xffffff;').Replace('return mItemOffsets[index];','return nativeSoundTableU32(reinterpret_cast<const u8*>(mItemOffsets)+index*4);')
$h=$h.Replace('void init(void const*);'+[Environment]::NewLine+'    u8 getTypeID', 'u32 nativeResourceSize=0;'+[Environment]::NewLine+'    void init(void const*);'+[Environment]::NewLine+'    u8 getTypeID')
Set-Content "$n\include\JSystem\JAudio2\JAUSoundTable.hpp" $h
$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JAUSoundTable.cpp" -Raw
$source=$source.Replace('void JAUSoundTable::init(void const* pData) {', 'void JAUSoundTable::init(void const* pData) { nativeResourceSize=0;')
$source=$source.Replace('return section->mNumGroups;','return nativeSoundTableU32(&section->mNumGroups);').Replace('return group->mNumItems;','return nativeSoundTableU32(&group->mNumItems);')
Set-Content "$n\src\generated\SoundTableKernel.cpp" $source
$source=Get-Content "$p\Petari\src\Game\AudioLib\AudSoundInfo.cpp" -Raw
$source=$source.Replace('reinterpret_cast< JAUSoundTableBgm* >(data)->mSoundSw','nativeSoundTableU16(static_cast<const u8*>(data)+6)').Replace('reinterpret_cast< JAUSoundTableSe* >(data)->mSoundSw','nativeSoundTableU16(static_cast<const u8*>(data)+4)').Replace('reinterpret_cast< JAUSoundTableBgm* >(data)->mChordResId','nativeSoundTableU16(static_cast<const u8*>(data)+4)')
$source=$source.Replace('void* data = JASGlobalInstance< JAUSoundTable >::getInstance()->getData(soundID);','const auto* table=JASGlobalInstance< JAUSoundTable >::getInstance();'+[Environment]::NewLine+'    void* data = table ? table->getData(soundID) : nullptr;').Replace('u8 typeID = JASGlobalInstance< JAUSoundTable >::getInstance()->getTypeID(soundID);','u8 typeID = table ? table->getTypeID(soundID) : 0xff;')
Set-Content "$n\src\generated\SoundInfoKernel.cpp" $source

