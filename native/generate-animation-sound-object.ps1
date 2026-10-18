$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
New-Item -ItemType Directory -Force "$n\include\JSystem\JAudio2" | Out-Null
$waveHeader=Get-Content "$p\Petari\libs\JSystem\include\JSystem\JAudio2\JASWaveArcLoader.hpp" -Raw
$waveHeader=$waveHeader.Replace('void setFileName(char const*) ATTRIBUTE_WEAK;','void setFileName(char const*);').Replace('bool load(JASHeap*) ATTRIBUTE_WEAK;','bool load(JASHeap*);')
Set-Content "$n\include\JSystem\JAudio2\JASWaveArcLoader.hpp" $waveHeader
$header=Get-Content "$p\Petari\libs\JSystem\include\JSystem\JAudio2\JAISoundHandles.hpp" -Raw
Set-Content "$n\include\JSystem\JAudio2\JAISoundHandles.hpp" ('#include <cstdint>'+[Environment]::NewLine+$header.Replace('getHandleUserData(u32)','getHandleUserData(std::uintptr_t)'))
$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JAISoundHandles.cpp" -Raw
Set-Content "$n\src\generated\SoundHandlesKernel.cpp" ($source.Replace('getHandleUserData(u32 addr)','getHandleUserData(std::uintptr_t addr)').Replace('(u32)mHandles[i]->getUserData()','mHandles[i]->getUserData()'))
$source=Get-Content "$p\Petari\src\Game\AudioLib\AudAnmSoundObject.cpp" -Raw
Set-Content "$n\src\generated\AnimationSoundObjectKernel.cpp" ($source.Replace('(u32)sound','reinterpret_cast<std::uintptr_t>(sound)'))
$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JAUSoundObject.cpp" -Raw
Set-Content "$n\src\generated\JAUSoundObjectKernel.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$source.Replace('== FALSE','== 0'))
$source=Get-Content "$p\Petari\src\Game\AudioLib\AudSoundObjHolder.cpp" -Raw
Set-Content "$n\src\generated\SoundObjectHolderKernel.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$source.Replace('== FALSE','== 0'))
$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JASBasicInst.cpp" -Raw
$source=$source.Replace('delete[] mVelomap;','if (mVelomap) JKRHeap::free(mVelomap, nullptr);')
$source=$source.Replace('delete[] mKeymap;','if (mKeymap) { for (int i=0;i<mKeymapCount;++i) mKeymap[i].~TKeymap(); JKRHeap::free(mKeymap, nullptr); } mKeymap=nullptr; mKeymapCount=0;')
# Explicit object construction avoids an implementation-dependent array cookie.
$source=$source.Replace('mKeymap = new (pHeap, 0) TKeymap[count];','if (count > std::numeric_limits<u32>::max()/sizeof(TKeymap)) throw std::bad_alloc();'+[Environment]::NewLine+'    mKeymap = count ? static_cast<TKeymap*>(JKRHeap::alloc(count*sizeof(TKeymap), alignof(TKeymap), pHeap)) : nullptr;'+[Environment]::NewLine+'    if (count && !mKeymap) { mKeymapCount=0; throw std::bad_alloc(); }'+[Environment]::NewLine+'    for (u32 i=0;i<count;++i) new (mKeymap+i) TKeymap();')
Set-Content "$n\src\generated\BasicInstrumentKernel.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$source.Replace('== FALSE','== 0'))
$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JASBasicBank.cpp" -Raw
Set-Content "$n\src\generated\BasicBankKernel.cpp" ('#include <cstring>'+[Environment]::NewLine+$source.Replace('JASCalc::bzero(&mInstTable, INST_NUM_MAX * 4);','std::memset(mInstTable, 0, sizeof(mInstTable));'))
$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JASCalc.cpp" -Raw
Set-Content "$n\src\generated\AudioCalcKernel.cpp" ('#include <cstdint>'+[Environment]::NewLine+$source.Replace('reinterpret_cast< u32 >','reinterpret_cast< std::uintptr_t >'))
foreach($unit in @('JASSimpleWaveBank','JASBasicWaveBank','JASWaveArcLoader')){
 $source=Get-Content "$p\Petari\src\JSystem\JAudio2\$unit.cpp" -Raw
 if($unit -eq 'JASWaveArcLoader'){$source=$source.Replace('sendCmdMsg(loadToAramCallback, &commandInfo, 0x10)','sendCmdMsg(loadToAramCallback, &commandInfo, sizeof(commandInfo))').Replace('delete[] path;','JKRHeap::free(path,nullptr);').Replace('new (JASKernel::getSystemHeap(), -4) char','new (JASKernel::getSystemHeap(), -int(alignof(std::max_align_t))) char')}
 if($unit -eq 'JASSimpleWaveBank'){
  $source=$source.Replace('delete[] mWaveTable;','if(mWaveTable){for(u32 i=0;i<mWaveTableSize;++i)mWaveTable[i].~TWaveHandle();JKRHeap::free(mWaveTable,nullptr);}mWaveTable=nullptr;mWaveTableSize=0;')
  $source=$source.Replace('mWaveTable = new (heap, 0) TWaveHandle[size];','if(size>std::numeric_limits<u32>::max()/sizeof(TWaveHandle))throw std::bad_alloc();'+[Environment]::NewLine+'    mWaveTable=size?static_cast<TWaveHandle*>(JKRHeap::alloc(size*sizeof(TWaveHandle),alignof(TWaveHandle),heap)):nullptr;'+[Environment]::NewLine+'    if(size&&!mWaveTable)throw std::bad_alloc();'+[Environment]::NewLine+'    for(u32 i=0;i<size;++i)new(mWaveTable+i)TWaveHandle();')
 }
 if($unit -eq 'JASBasicWaveBank'){
  $source=$source.Replace('delete[] mWaveTable;','if(mWaveTable)JKRHeap::free(mWaveTable,nullptr);mWaveTable=nullptr;mHandleCount=0;')
  $source=$source.Replace('delete mWaveGroupArray[i];','if(mWaveGroupArray[i]){mWaveGroupArray[i]->~TWaveGroup();JKRHeap::free(mWaveGroupArray[i],nullptr);}')
  $source=$source.Replace('delete[] mWaveGroupArray;','if(mWaveGroupArray)JKRHeap::free(mWaveGroupArray,nullptr);mWaveGroupArray=nullptr;mGroupCount=0;')
  $source=$source.Replace('delete[] mCtrlWaveArray;','if(mCtrlWaveArray){for(u32 i=0;i<mWaveCount;++i)mCtrlWaveArray[i].~TWaveInfo();JKRHeap::free(mCtrlWaveArray,nullptr);}mCtrlWaveArray=nullptr;mWaveCount=0;')
  $source=$source.Replace('mWaveCount = (u16)count;','mWaveCount = count;').Replace('mGroupCount = (u16)param_0;','mGroupCount = param_0;').Replace('mHandleCount = (u16)count;','mHandleCount = count;')
  $source=$source.Replace('mGroupCount = param_0;','mGroupCount = 0;')
  $source=$source.Replace('mWaveGroupArray = new (heap, 0) TWaveGroup*[param_0];','if(param_0>std::numeric_limits<u32>::max()/sizeof(TWaveGroup*))throw std::bad_alloc();'+[Environment]::NewLine+'    mWaveGroupArray=param_0?new(heap,0)TWaveGroup*[param_0]():nullptr;')
  $source=$source.Replace('for (int i = 0; i < mGroupCount; i++) {'+[Environment]::NewLine+'        mWaveGroupArray[i] = new (heap, 0) TWaveGroup(this);','for (u32 i = 0; i < param_0; i++) {'+[Environment]::NewLine+'        mWaveGroupArray[i] = new (heap, 0) TWaveGroup(this);'+[Environment]::NewLine+'        ++mGroupCount;')
  $source=$source.Replace('mCtrlWaveArray = new (heap, 0) TWaveInfo[count];','if(count>std::numeric_limits<u32>::max()/sizeof(TWaveInfo))throw std::bad_alloc();'+[Environment]::NewLine+'    mCtrlWaveArray=count?static_cast<TWaveInfo*>(JKRHeap::alloc(count*sizeof(TWaveInfo),alignof(TWaveInfo),heap)):nullptr;'+[Environment]::NewLine+'    if(count&&!mCtrlWaveArray){mWaveCount=0;throw std::bad_alloc();}'+[Environment]::NewLine+'    for(u32 i=0;i<count;++i)new(mCtrlWaveArray+i)TWaveInfo();')
 }
 Set-Content "$n\src\generated\$($unit)Kernel.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$source.Replace('== FALSE','== 0'))
}
$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JASHeapCtrl.cpp" -Raw
$source=$source.Replace('(u8*)OSRoundUp32B(pBase)','reinterpret_cast<u8*>((reinterpret_cast<std::uintptr_t>(pBase)+31)&~std::uintptr_t(31))').Replace('(u32(mBase) - u32(pBase))','static_cast<u32>(mBase-static_cast<u8*>(pBase))').Replace('u32(it->mBase) - u32(pCurrent)','static_cast<u32>(it->mBase-pCurrent)').Replace('delete[] pChunk;','delete[] static_cast<u8*>(pChunk);')
Set-Content "$n\src\generated\AudioHeapKernel.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+'#include <cstdint>'+[Environment]::NewLine+$source)
$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JASTaskThread.cpp" -Raw
$source=$source.Replace('u32 size = msgSize + 8;','if(!callback || (msgSize && !msg) || msgSize > 1024 - offsetof(JASThreadCallStack, msg)) return nullptr;'+[Environment]::NewLine+'    u32 size = u32(offsetof(JASThreadCallStack, msg)) + msgSize;').Replace('getCommandHeap()->alloc(12)','getCommandHeap()->alloc(u32(offsetof(JASThreadCallStack, msg) + sizeof(void*)))')
Set-Content "$n\src\generated\AudioTaskThreadKernel.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+'#include <cstddef>'+[Environment]::NewLine+$source)
$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JASDvdThread.cpp" -Raw
Set-Content "$n\src\generated\AudioDvdThreadKernel.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$source)

$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JASDSPChannel.cpp" -Raw
Set-Content "$n\src\generated\AudioDSPChannelKernel.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$source)










$source=Get-Content "$p\Petari\src\JSystem\JAudio2\JASAudioThread.cpp" -Raw
Set-Content "$n\src\generated\AudioThreadKernel.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$source)

