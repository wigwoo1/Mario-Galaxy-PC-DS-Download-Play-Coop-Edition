$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$h=Get-Content "$p\Petari\libs\JSystem\include\JSystem\JAudio2\JASBNKParser.hpp" -Raw
$h=$h.Replace('namespace JASBNKParser {','inline u32 nativeBNKU32(const void* ptr){const auto* b=static_cast<const u8*>(ptr);return (u32(b[0])<<24)|(u32(b[1])<<16)|(u32(b[2])<<8)|b[3];}'+[Environment]::NewLine+'namespace JASBNKParser {').Replace('stream, offset','stream, nativeBNKU32(&offset)').Replace('return ptr[2];','return nativeBNKU32(static_cast<const u8*>(param_0)+8);')
Set-Content "$n\include\JSystem\JAudio2\JASBNKParser.hpp" $h
