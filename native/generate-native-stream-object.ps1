$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JAIStream.cpp" -Raw
$s='#include <cstdint>'+[Environment]::NewLine+'#include <stdexcept>'+[Environment]::NewLine+$s
$s=$s.Replace('inner_.aramStream.init((u32)streamAramAddr, local_28, &JAIStream_JASAramStreamCallback_, this);','const auto address = reinterpret_cast<uintptr_t>(streamAramAddr);'+[Environment]::NewLine+'            if (address > UINT32_MAX) throw std::out_of_range("Stream ARAM address exceeds virtual address space");'+[Environment]::NewLine+'            inner_.aramStream.init(u32(address), local_28, &JAIStream_JASAramStreamCallback_, this);')
Set-Content "$n\src\generated\JAIStream.cpp" $s
