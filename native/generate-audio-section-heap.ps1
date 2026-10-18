$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JAUSectionHeap.cpp" -Raw
$s=$s.Replace('soundTable->init(bstDst);', 'soundTable->init(bstDst); soundTable->nativeResourceSize = param_1;')
Set-Content "$n\src\generated\AudioSectionHeapKernel.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$s)

