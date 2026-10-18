$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content -Raw "$p\Petari\src\Game\Util\MemoryUtil.cpp"
$start=$s.IndexOf('    void copyMemory(');$brace=$s.IndexOf('{',$start);$depth=1;$end=$brace+1
while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++}
$body=$s.Substring($start,$end-$start).Replace('reinterpret_cast< u32 >(pSrc)','reinterpret_cast< uintptr_t >(pSrc)').Replace('reinterpret_cast< u32 >(pDst)','reinterpret_cast< uintptr_t >(pDst)')
$body=$body.Replace('srcAlignment == dstAlignment && (size & 0xF) == 0','srcAlignment == 0 && dstAlignment == 0 && (size & 0xF) == 0')
$body=$body.Replace('*pSrcWords++','loadWord(pSrcWords++)')
foreach($i in 0..3){$body=$body.Replace('*pDstWords++ = word'+$i+';','storeWord(pDstWords++,word'+$i+');')}
$body=$body.Replace('*reinterpret_cast< u32* >(pDstBytes) = *reinterpret_cast< const u32* >(pSrcBytes);','std::memcpy(pDstBytes,pSrcBytes,4);')
Set-Content "$n\src\generated\MemoryCopyKernel.cpp" ('#include <revolution/types.h>'+"`n"+'#include <cstdint>'+"`n"+'#include <cstring>'+"`n"+'namespace MR {static u32 loadWord(const void* source){u32 word;std::memcpy(&word,source,4);return word;}static void storeWord(void* destination,u32 word){std::memcpy(destination,&word,4);}'+"`n"+$body+"`n}`n")
