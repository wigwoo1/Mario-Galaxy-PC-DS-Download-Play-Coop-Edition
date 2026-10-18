$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\DrawUtil.cpp" -Raw
$body=@('#include <Game/Util/DrawUtil.hpp>','#include <Game/Util/Color.hpp>','#include <JSystem/J3DGraphAnimator/J3DJoint.hpp>','#include <JSystem/J3DGraphAnimator/J3DModelData.hpp>','#include <JSystem/J3DGraphBase/J3DMaterial.hpp>','#include <JSystem/J3DGraphBase/J3DShape.hpp>','#include <JSystem/J3DGraphBase/J3DSys.hpp>','#include <revolution/gx.h>','namespace MR {')
foreach($sig in @('    void setupShadowVolumeDraw(','    void drawSimpleModel(')){
 $a=$s.IndexOf($sig);if($a -lt 0){throw "Missing draw anchor $sig"};$b=$s.IndexOf('{',$a);$d=1;$e=$b+1;while($d){if($s[$e] -eq '{'){$d++}elseif($s[$e] -eq '}'){$d--};$e++};$body+=$s.Substring($a,$e-$a)
}
$body+='}'
Set-Content "$n\src\generated\ShadowDrawKernel.cpp" ($body -join [Environment]::NewLine)
