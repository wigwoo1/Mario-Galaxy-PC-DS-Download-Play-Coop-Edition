$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out='#include <Game/Util/MtxUtil.hpp>'+"`r`n"+'#include <Game/Util/MathUtil.hpp>'+"`r`n"+'#include <cstring>'+"`r`nnamespace MR {`r`n"
foreach($entry in @(@('MtxUtil','void setMtxTrans('),@('MtxUtil','void extractMtxTrans('),@('StringUtil','void extractString('),@('MathUtil','void vecBlend('))){
$s=Get-Content "$p\Petari\src\Game\Util\$($entry[0]).cpp" -Raw
$start=$s.IndexOf($entry[1]);if($start -lt 0){throw 'Original helper signature missing'}
$open=$s.IndexOf('{',$start);$depth=1;$end=$open+1
while($depth){if($s[$end] -eq '{'){$depth++};if($s[$end] -eq '}'){$depth--};$end++}
$out+=$s.Substring($start,$end-$start)+"`r`n"
}
$out+="}`r`n"
Set-Content "$n\src\generated\AnimationHelperKernel.cpp" $out -Encoding UTF8

