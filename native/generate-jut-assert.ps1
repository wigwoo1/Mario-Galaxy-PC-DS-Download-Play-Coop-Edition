$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JUtility\JUTAssert.cpp" -Raw
Set-Content "$n\src\generated\JUTAssertKernel.cpp" ('#include <cstdarg>'+[Environment]::NewLine+'#define __va_list va_list'+[Environment]::NewLine+$s)
