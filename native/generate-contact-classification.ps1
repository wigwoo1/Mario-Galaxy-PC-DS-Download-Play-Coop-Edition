$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\MapUtil.cpp" -Raw
$out='#include <Game/Util/MapUtil.hpp>'+[Environment]::NewLine+'#include <Game/Util/MathUtil.hpp>'+[Environment]::NewLine+'namespace MR {'
foreach($signature in @('bool isWallPolygon(const TVec3f&','bool isFloorPolygon(const TVec3f&','bool isFloorPolygonCos(','bool isWallPolygon(f32','bool isFloorPolygon(f32')){
 $b=$s.IndexOf($signature);if($b -lt 0){throw "Missing original contact classification $signature"};$o=$s.IndexOf('{',$b);$e=$o+1;$d=1
 while($d){if($s[$e] -eq '{'){$d++}elseif($s[$e] -eq '}'){$d--};$e++}
 $out+=$s.Substring($b,$e-$b)+[Environment]::NewLine
}
Set-Content "$n\src\generated\CollisionContactClass.cpp" ($out+'}')
