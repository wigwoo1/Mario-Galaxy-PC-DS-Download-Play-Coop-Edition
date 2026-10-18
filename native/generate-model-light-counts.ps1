$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\ModelUtil.cpp" -Raw
function Extract($sig){$a=$s.IndexOf($sig);if($a -lt 0){throw "Missing original $sig"};$o=$s.IndexOf('{',$a);$d=1;$e=$o+1;while($d){if($s[$e] -eq '{'){$d++}elseif($s[$e] -eq '}'){$d--};$e++};return $s.Substring($a,$e-$a)}
$out=@('#include <Game/Util/ModelUtil.hpp>','#include <JSystem/J3DGraphAnimator/J3DModel.hpp>','#include <JSystem/J3DGraphBase/J3DMaterial.hpp>','namespace {',(Extract 'inline bool isUseLightChanNo('),'}','namespace MR {')
foreach($sig in @('void getLightNum(J3DMaterial*','s32 getLightNum(J3DMaterial*','s32 getLightNumMax(const LiveActor*','s32 getLightNumMax(J3DModelData*')){$out+=Extract $sig}
$out+='}'
Set-Content "$n\src\generated\ModelLightCounts.cpp" ($out -join "`n")
