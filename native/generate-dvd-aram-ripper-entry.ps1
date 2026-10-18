$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JKernel\JKRDvdAramRipper.cpp" -Raw
$start=$s.IndexOf('JKRAramBlock* JKRDvdAramRipper::loadToAram')
$end=$s.IndexOf('JKRADCommand* JKRDvdAramRipper::loadToAram_Async',$start)
$s=$s.Substring($start,$end-$start)
$s=$s.Replace('syncAram(command, 0);','if (!command) return nullptr;'+[Environment]::NewLine+'    syncAram(command, 0);').Replace('delete command;','command->~JKRADCommand(); JKRHeap::free(command, nullptr);')
$prefix=@"
#include <Native/HeapPlacement.hpp>
#include <JSystem/JKernel/JKRDvdAramRipper.hpp>
#include <JSystem/JKernel/JKRDvdFile.hpp>
"@
Set-Content "$n\src\generated\DvdAramRipperEntryKernel.cpp" ($prefix+[Environment]::NewLine+$s)
