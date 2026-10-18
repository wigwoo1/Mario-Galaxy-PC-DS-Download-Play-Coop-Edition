$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JUtility\JUTConsole.cpp" -Raw
$anchor='#include "JSystem/JKernel/JKRHeap.hpp"'
if(!$s.Contains($anchor)){throw 'Original console heap include anchor missing'}
$s=$s.Replace($anchor,'#include <Native/HeapPlacement.hpp>')
Set-Content "$n\src\generated\JUTConsole.cpp" $s
$lines=Get-Content "$n\src\jut_console_port.cpp" | Where-Object {$_ -notmatch '^extern "C" void JUTReportConsole'};Set-Content "$n\src\generated\JUTConsoleHostDiagnostics.cpp" $lines
