$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content -Raw "$p\Petari\src\Game\Util\ModelUtil.cpp"
$start=$s.IndexOf('    bool isUseFur(');if($start -lt 0){throw 'Original fur check missing'}
$brace=$s.IndexOf('{',$start);$depth=1;$end=$brace+1;while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++}
$body=$s.Substring($start,$end-$start)
foreach($name in @('invalidateMtxCalc','invalidateJointCallback')){
 $start=$s.IndexOf('    void '+$name+'(');if($start -lt 0){throw "Original model reset missing: $name"}
 $brace=$s.IndexOf('{',$start);$depth=1;$end=$brace+1;while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++}
 $body+="`n"+$s.Substring($start,$end-$start)
}
Set-Content "$n\src\generated\ModelFurCheck.cpp" ('#include <JSystem/J3DGraphAnimator/J3DModelData.hpp>'+"`n"+'#include <JSystem/JUtility/JUTNameTab.hpp>'+"`n"+'#include <JSystem/J3DGraphAnimator/J3DJoint.hpp>'+"`n"+'#include <cstring>'+"`n"+'namespace MR {'+"`n"+$body+"`n}`n")



