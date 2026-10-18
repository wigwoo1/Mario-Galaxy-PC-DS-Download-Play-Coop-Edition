$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JASDrumSet.cpp" -Raw
# TVelomap is trivial: free original heap-owned array through heap owner lookup.
$s=$s.Replace('delete[] mVelomap;','if (mVelomap) JKRHeap::free(mVelomap, nullptr);')
Set-Content "$n\src\generated\DrumSetKernel.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$s)
