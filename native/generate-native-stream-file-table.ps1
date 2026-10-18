$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JAUStreamFileTable.cpp" -Raw
$s=$s.Replace('#include <revolution/dvd.h>','#include <revolution/dvd.h>'+[Environment]::NewLine+'namespace { u32 streamTableWord(const void* value){auto p=static_cast<const u8*>(value);return u32(p[0])<<24|u32(p[1])<<16|u32(p[2])<<8|p[3];} }')
$s=$s.Replace('return mData->mNumFiles;', 'return mData ? streamTableWord(reinterpret_cast<const u8*>(mData)+4) : 0;')
$s=$s.Replace('if (mData == nullptr) {','if (mData == nullptr || index < 0 || u32(index) >= getNumFiles()) {')
$s=$s.Replace('*(int*)(data + 8 + (index * sizeof(s32)))','streamTableWord(data + 8 + (index * sizeof(s32)))')
Set-Content "$n\src\generated\JAUStreamFileTable.cpp" $s
