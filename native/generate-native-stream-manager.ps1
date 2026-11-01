$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$h=Get-Content "$p\Petari\libs\JSystem\include\JSystem\JAudio2\JAIStreamDataMgr.hpp" -Raw
$h=$h.Replace('#pragma once','#pragma once'+[Environment]::NewLine+'#include <cstdint>').Replace('deleteStreamAram(u32)','deleteStreamAram(uintptr_t)')
Set-Content "$n\include\JSystem\JAudio2\JAIStreamDataMgr.hpp" $h
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JAIStreamMgr.cpp" -Raw
$s=$s.Replace('deleteStreamAram((u32)aramAddr)','deleteStreamAram(reinterpret_cast<uintptr_t>(aramAddr))')
$success='    return false;(\r?\n\}\r?\n\r?\nvoid JAIStreamMgr::freeDeadStream_\(\))'
if([regex]::Matches($s,$success).Count-ne 1){throw 'Original stream start success return anchor changed'}
$s=[regex]::Replace($s,$success,'    return true;$1')
Set-Content "$n\src\generated\JAIStreamMgr.cpp" $s
