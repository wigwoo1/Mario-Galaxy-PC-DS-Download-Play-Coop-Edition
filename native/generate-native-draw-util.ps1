$ErrorActionPreference='Stop'
$n=$PSScriptRoot
function Remove-Definition([string]$source,[string]$signature){
 $start=$source.IndexOf($signature);if($start -lt 0){throw "Missing definition $signature"}
 $open=$source.IndexOf('{',$start);$depth=1;$end=$open+1
 while($depth -gt 0 -and $end -lt $source.Length){if($source[$end] -eq '{'){$depth++};if($source[$end] -eq '}'){$depth--};$end++}
 if($depth){throw "Unbalanced definition $signature"}
 return $source.Remove($start,$end-$start)
}
$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\DrawUtil.cpp" -Raw
foreach($signature in @('void loadProjectionMtxFor2D()','void setupShadowVolumeDraw()','void drawSimpleModel(J3DModelData* pModelData)')){
 $s=Remove-Definition $s $signature
}
Set-Content "$n\src\generated\DrawUtil.cpp" $s
