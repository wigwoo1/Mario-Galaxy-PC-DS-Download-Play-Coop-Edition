$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$p=Split-Path $n -Parent
$source=Get-Content -LiteralPath "$p\Petari\src\RVL_SDK\mtx\quat.c" -Raw
function ExtractOriginalDefinition([string]$text,[string]$signature){
    $start=$text.IndexOf($signature,[StringComparison]::Ordinal)
    if($start -lt 0){throw "Missing original quaternion method: $signature"}
    $open=$text.IndexOf('{',$start)
    if($open -lt 0){throw "Missing body for original quaternion method: $signature"}
    $depth=0
    for($i=$open;$i -lt $text.Length;$i++){
        if($text[$i] -eq '{'){$depth++}
        elseif($text[$i] -eq '}'){
            $depth--
            if($depth -eq 0){return $text.Substring($start,$i-$start+1)}
        }
    }
    throw "Unclosed body for original quaternion method: $signature"
}
$methods=@(
    (ExtractOriginalDefinition $source 'void C_QUATMtx('),
    (ExtractOriginalDefinition $source 'void C_QUATSlerp(')
)
$generated=@'
#include "revolution/mtx.h"
#include <cmath>
static const f32 one[] = {1.0f};
'@
Set-Content -LiteralPath "$n\src\generated\QuaternionSdkHelpers.cpp" -Value ($generated+"`n"+($methods -join "`n`n")) -NoNewline
