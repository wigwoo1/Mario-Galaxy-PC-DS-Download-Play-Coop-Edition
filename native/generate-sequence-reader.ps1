$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\libs\JSystem\include\JSystem\JAudio2\JASSeqReader.hpp" -Raw
function ReplaceReader([string]$signature,[string]$body){
 $start=$script:s.IndexOf($signature);if($start -lt 0){throw "Missing reader function $signature"};$open=$script:s.IndexOf('{',$start);$depth=1;$end=$open+1
 while($depth){if($script:s[$end] -eq '{'){$depth++}elseif($script:s[$end] -eq '}'){$depth--};$end++}
 $script:s=$script:s.Substring(0,$open)+'{'+$body+'}'+$script:s.Substring($end)
}
ReplaceReader 'u32 get24(' 'return (u32(mSeqBuff[param_0])<<16)|(u32(mSeqBuff[param_0+1])<<8)|mSeqBuff[param_0+2];'
ReplaceReader 'u16 get16(' 'return (u16(mSeqBuff[param_0])<<8)|mSeqBuff[param_0+1];'
ReplaceReader 'u32 get32(' 'return (u32(get16(param_0))<<16)|get16(param_0+2);'
ReplaceReader 'u32 getOffset(' 'return static_cast<u32>(mSeqCursor-mSeqBuff);'
ReplaceReader 'u32 read16(' 'const u32 value=get16(getOffset());mSeqCursor+=2;return value;'
ReplaceReader 'u32 read24(' 'const u32 value=get24(getOffset());mSeqCursor+=3;return value;'
Set-Content "$n\include\JSystem\JAudio2\JASSeqReader.hpp" $s
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JASSeqReader.cpp" -Raw
Set-Content "$n\src\generated\SequenceReaderKernel.cpp" ($s.Replace('mLoopCounts[i] = nullptr;','mLoopCounts[i] = 0;'))
