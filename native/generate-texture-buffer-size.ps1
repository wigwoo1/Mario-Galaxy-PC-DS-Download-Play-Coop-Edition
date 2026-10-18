$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$s=Get-Content -Raw "$p\Petari\src\RVL_SDK\gx\GXTexture.c"
$body=''
foreach($signature in @('static void __GXGetTexTileShift(','u32 GXGetTexBufferSize(')){
 $start=$s.IndexOf($signature);if($start -lt 0){throw 'Original texture sizing anchor missing'};$brace=$s.IndexOf('{',$start);$depth=1;$end=$brace+1;while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++};$body+=$s.Substring($start,$end-$start)+"`n"
}
Set-Content "$n\src\generated\TextureBufferSizeKernel.cpp" ('#include <revolution/gx.h>'+"`n"+$body)
