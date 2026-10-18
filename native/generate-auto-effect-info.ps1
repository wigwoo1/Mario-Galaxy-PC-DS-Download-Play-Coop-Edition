$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Effect\AutoEffectInfo.cpp" -Raw
$anchor='extern "C" u32 strtoul(const char*, char**, int);'
if(!$s.Contains($anchor)){throw 'Original strtoul declaration missing'}
$s=$s.Replace($anchor,'#include <cstdlib>')
Set-Content "$n\src\generated\AutoEffectInfo.cpp" $s
