$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$h=Get-Content "$p\Petari\libs\JSystem\include\JSystem\JAudio2\JASWSParser.hpp" -Raw
$helpers=@"
#include <cstring>
inline u32 nativeWSU32(const void* ptr){const auto* b=static_cast<const u8*>(ptr);return (u32(b[0])<<24)|(u32(b[1])<<16)|(u32(b[2])<<8)|b[3];}
inline s16 nativeWSS16(const void* ptr){const auto* b=static_cast<const u8*>(ptr);return static_cast<s16>((u16(b[0])<<8)|b[1]);}
inline f32 nativeWSFloat(const void* ptr){u32 bits=nativeWSU32(ptr);f32 result;std::memcpy(&result,&bits,4);return result;}
"@
$h=$h.Replace('class JKRHeap;',$helpers+[Environment]::NewLine+'class JKRHeap;').Replace('param_0, mOffset','param_0, nativeWSU32(&mOffset)')
Set-Content "$n\include\JSystem\JAudio2\JASWSParser.hpp" $h
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JASWSParser.cpp" -Raw
$s=[regex]::Replace($s,'\b(\w+)->(mGroupCount|mWaveCount|mData|mAWStartOffs|mAWLength|mLoopFlags|mSampleLoopStart|mSampleLoopEnd|mSampleCount)\b','nativeWSU32(&$1->$2)')
$s=[regex]::Replace($s,'\b(\w+)->(mpLastSample|mpPenultSample)\b','nativeWSS16(&$1->$2)')
$s=$s.Replace('wave->mSampleRate','nativeWSFloat(&wave->mSampleRate)')
Set-Content "$n\src\generated\WaveSystemParserKernel.cpp" ('#include <Native/HeapPlacement.hpp>'+[Environment]::NewLine+$s)
