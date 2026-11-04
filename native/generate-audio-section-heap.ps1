$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JAUSectionHeap.cpp" -Raw
$s=$s.Replace('soundTable->init(bstDst);', 'soundTable->init(bstDst); soundTable->nativeResourceSize = param_1;')
# Wii default new uses the selected JKR heap; native default new uses the CRT.
# Keep catalog, section and sequence metadata in their original selected heaps.
foreach($allocation in @(
 @{Old='new JAUSoundTable(param_2)';New='new (getHeap_(), alignof(JAUSoundTable)) JAUSoundTable(param_2)';Count=1},
 @{Old='new JAUSoundNameTable(param_2)';New='new (getHeap_(), alignof(JAUSoundNameTable)) JAUSoundNameTable(param_2)';Count=1},
 @{Old='new JAUSeqDataMgr_SeqCollection()';New='new (getHeap_(), alignof(JAUSeqDataMgr_SeqCollection)) JAUSeqDataMgr_SeqCollection()';Count=1},
 @{Old='new JAUSectionHeap(heap, param_1, r29)';New='new (heap, alignof(JAUSectionHeap)) JAUSectionHeap(heap, param_1, r29)';Count=1},
 @{Old='new JAUSeqDataBlock()';New='new (getHeap_(), alignof(JAUSeqDataBlock)) JAUSeqDataBlock()';Count=2},
 @{Old='new JSULink< JAUSeqDataBlock >(seqDataBlock)';New='new (getHeap_(), alignof(JSULink< JAUSeqDataBlock >)) JSULink< JAUSeqDataBlock >(seqDataBlock)';Count=2}
)){
 if([regex]::Matches($s,[regex]::Escape($allocation.Old)).Count-ne $allocation.Count){throw 'Original sequence metadata allocation anchors changed'}
 $s=$s.Replace($allocation.Old,$allocation.New)
}
Set-Content "$n\src\generated\AudioSectionHeapKernel.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$s)

