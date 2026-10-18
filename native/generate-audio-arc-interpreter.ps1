$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$h=Get-Content "$p\Petari\libs\JSystem\include\JSystem\JAudio2\JAUAudioArcInterpreter.hpp" -Raw
$h=$h.Replace('u32 temp = *(u32*)mReadPtr;','u32 temp = (u32(mReadPtr[0])<<24)|(u32(mReadPtr[1])<<16)|(u32(mReadPtr[2])<<8)|mReadPtr[3];')
$h='#include <cstddef>'+[Environment]::NewLine+'#include <stdexcept>'+[Environment]::NewLine+$h
$h=$h.Replace('bool parse(void const*);', 'bool parse(void const*);'+[Environment]::NewLine+'    std::size_t nativeArchiveSize=0; bool nativeBounded=false;'+[Environment]::NewLine+'    bool parse(const void* data,std::size_t size){if(!data || size<8)return false;nativeArchiveSize=size;nativeBounded=true;try{bool result=parse(data);nativeBounded=false;return result;}catch(const std::out_of_range&){nativeBounded=false;return false;}}'+[Environment]::NewLine+'    void nativeCheckRead(std::size_t count) const {if(nativeBounded){const auto offset=std::size_t(mReadPtr-mBase);if(offset>nativeArchiveSize || count>nativeArchiveSize-offset)throw std::out_of_range("Audio archive command extent");}}')
$h=$h.Replace('return *mReadPtr++;','nativeCheckRead(1); return *mReadPtr++;').Replace('u32 temp = (u32(mReadPtr[0])', 'nativeCheckRead(4); u32 temp = (u32(mReadPtr[0])')
$h=$h.Replace('return mBase + offset;', 'if(nativeBounded && offset>nativeArchiveSize)throw std::out_of_range("Audio archive resource offset");return mBase + offset;')
Set-Content "$n\include\JSystem\JAudio2\JAUAudioArcInterpreter.hpp" $h
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JAUAudioArcInterpreter.cpp" -Raw
$s=$s.Replace('mBase = (const u8*)mData;','if (!mData) return false;'+[Environment]::NewLine+'    mBase = (const u8*)mData;')
foreach($pair in @(@('var1','var2','readBSC'),@('var1','var2','readBST'),@('var1','var2','readBSTN'))){$call=$pair[2]+'(getContent_(var1), var2 - var1);';$s=$s.Replace($call,'if (var2 < var1) throw std::out_of_range("Reversed audio resource extent"); getContent_(var2); '+$call)}
$s=$s.Replace('readBMS(var1, getContent_(var2), var3 - var2);','if (var3 < var2) throw std::out_of_range("Reversed sequence extent"); getContent_(var3); readBMS(var1, getContent_(var2), var3 - var2);')
Set-Content "$n\src\generated\AudioArcInterpreterKernel.cpp" $s

