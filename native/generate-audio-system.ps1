$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\AudioLib\AudSystem.cpp" -Raw
Set-Content "$n\src\generated\AudioSystemKernel.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$s)
$h=Get-Content "$p\Petari\libs\JSystem\include\JSystem\JAudio2\JAUStreamDataMgr.hpp" -Raw
$h=$h.Replace('#pragma once','#pragma once'+[Environment]::NewLine+'#include <cstdint>').Replace('addr == (u32)Base::mHeaps[i].mBase','std::uintptr_t(addr) == reinterpret_cast<std::uintptr_t>(Base::mHeaps[i].mBase)')
$h=$h.Replace('deleteStreamAram(u32', 'deleteStreamAram(uintptr_t')
Set-Content "$n\include\JSystem\JAudio2\JAUStreamDataMgr.hpp" $h

