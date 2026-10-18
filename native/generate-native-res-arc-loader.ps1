$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$h=Get-Content "$p\Petari\libs\JSystem\include\JSystem\JAudio2\JASResArcLoader.hpp" -Raw
$h=$h.Replace('#pragma once','#pragma once'+[Environment]::NewLine+'#include <cstdint>').Replace('(*LoadCallback)(u32, u32)','(*LoadCallback)(u32, uintptr_t)').Replace('u32 mCallbackArg','uintptr_t mCallbackArg').Replace('void (*)(u32, u32), u32','LoadCallback, uintptr_t')
Set-Content "$n\include\JSystem\JAudio2\JASResArcLoader.hpp" $h
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JASResArcLoader.cpp" -Raw
$s=$s.Replace('LoadCallback callback, u32 callbackArg','LoadCallback callback, uintptr_t callbackArg')
Set-Content "$n\src\generated\JASResArcLoader.cpp" $s
