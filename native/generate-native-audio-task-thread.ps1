$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JASTaskThread.cpp" -Raw
$s='#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+'#include <cstddef>'+[Environment]::NewLine+$s
$s=$s.Replace('u32 size = msgSize + 8;', 'const auto headerSize = offsetof(JASThreadCallStack, msg);'+[Environment]::NewLine+'    if (!callback || (msgSize && !msg) || msgSize > 1024 - headerSize) return nullptr;'+[Environment]::NewLine+'    u32 size = u32(headerSize + msgSize);')
$s=$s.Replace('callStack = (JASThreadCallStack*)JASKernel::getCommandHeap()->alloc(12);', 'if (!callback) return nullptr;'+[Environment]::NewLine+'    callStack = (JASThreadCallStack*)JASKernel::getCommandHeap()->alloc(offsetof(JASThreadCallStack, msg) + sizeof(void*));')
Set-Content "$n\src\generated\JASTaskThread.cpp" $s
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JASDvdThread.cpp" -Raw
Set-Content "$n\src\generated\JASDvdThread.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$s)

