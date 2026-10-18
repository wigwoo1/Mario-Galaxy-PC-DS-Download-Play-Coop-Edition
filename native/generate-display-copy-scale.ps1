$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$s=Get-Content -Raw "$p\Petari\src\RVL_SDK\gx\GXFrameBuf.c"
$body=''
foreach($signature in @('static u32 __GXGetNumXfbLines(','u16 GXGetNumXfbLines(','f32 GXGetYScaleFactor(')){
 $start=$s.IndexOf($signature);if($start -lt 0){throw 'Original display scaling anchor missing'};$brace=$s.IndexOf('{',$start);$depth=1;$end=$brace+1;while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++};$body+=$s.Substring($start,$end-$start)+"`n"
}
Set-Content "$n\src\generated\DisplayCopyScaleKernel.cpp" ('#include <revolution/gx.h>'+"`n"+$body)

