$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\AudioLib\AudSystem.cpp" -Raw
# Preserve original JKR default-new ownership locally on the native host.
foreach($allocation in @(
 @{Old='new AudSystem(pHeap, pChordArchive, pMeArchive, pRemixArchive)';New='new (pHeap->getHeap(), alignof(AudSystem)) AudSystem(pHeap, pChordArchive, pMeArchive, pRemixArchive)'},
 @{Old='new JAUDisposerObject_< JAUStdSoundInfo >(true)';New='new (heap->getHeap_(), alignof(JAUDisposerObject_< JAUStdSoundInfo >)) JAUDisposerObject_< JAUStdSoundInfo >(true)'},
 @{Old='new JAUStreamStaticAramMgr()';New='new (mSectionHeap->getHeap(), alignof(JAUStreamStaticAramMgr)) JAUStreamStaticAramMgr()'}
)){
 if([regex]::Matches($s,[regex]::Escape($allocation.Old)).Count-ne 1){throw 'Original AudSystem allocation anchor changed'}
 $s=$s.Replace($allocation.Old,$allocation.New)
}
$remixCall='        mRemixMgr->setRemixSeqResource(pRemixArchive->getResource(static_cast< u16 >(0)));'
if([regex]::Matches($s,[regex]::Escape($remixCall)).Count-ne 1){throw 'Original remix resource call anchor changed'}
$remixSized='        void* remixResource = pRemixArchive->getResource(static_cast< u16 >(0));'+[Environment]::NewLine+'        const s32 remixResourceSize = remixResource ? pRemixArchive->getResSize(remixResource) : 0;'+[Environment]::NewLine+'        mRemixMgr->setRemixSeqResource(remixResource, remixResourceSize > 0 ? u32(remixResourceSize) : 0);'
$s=$s.Replace($remixCall,$remixSized)
Set-Content "$n\src\generated\AudioSystemKernel.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$s)
$h=Get-Content "$p\Petari\libs\JSystem\include\JSystem\JAudio2\JAUStreamDataMgr.hpp" -Raw
$h=$h.Replace('#pragma once','#pragma once'+[Environment]::NewLine+'#include <cstdint>').Replace('addr == (u32)Base::mHeaps[i].mBase','std::uintptr_t(addr) == reinterpret_cast<std::uintptr_t>(Base::mHeaps[i].mBase)')
$h=$h.Replace('deleteStreamAram(u32', 'deleteStreamAram(uintptr_t')
Set-Content "$n\include\JSystem\JAudio2\JAUStreamDataMgr.hpp" $h

