$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\LiveActor\Binder.cpp" -Raw
$out='#include <Game/LiveActor/Binder.hpp>'+[Environment]::NewLine+'#include <Game/Map/HitInfo.hpp>'+[Environment]::NewLine+'#include <algorithm>'+[Environment]::NewLine+'#include <functional>'+[Environment]::NewLine+'#include <stdexcept>'+[Environment]::NewLine
foreach($signature in @('void Binder::setTriangleFilter(','void Binder::setCollisionPartsFilter(','const HitInfo* Binder::getPlane(','u32 Binder::copyPlaneArrayAndSortingSensor(','bool Binder::compSensor(')){
 $b=$s.IndexOf($signature);if($b -lt 0){throw "Missing original Binder method $signature"};$o=$s.IndexOf('{',$b);$e=$o+1;$d=1
 while($d){if($s[$e] -eq '{'){$d++}elseif($s[$e] -eq '}'){$d--};$e++}
 $method=$s.Substring($b,$e-$b)
 if($signature.StartsWith('u32 Binder::copyPlane')){
  $brace=$method.IndexOf('{')+1
  $method=$method.Insert($brace,' const u32 required=_24==0?u32(isBindedGround())+u32(isBindedWall())+u32(isBindedRoof()):mPlaneNum; if(required>capacity)throw std::length_error("Binder plane output capacity"); if(!required)return 0; if(!pPlanes)throw std::invalid_argument("Null Binder plane output"); ')
 }
 $method=$method.Replace('pPlane1->mParentTriangle.mSensor > pPlane2->mParentTriangle.mSensor','std::less<HitSensor*>{}(pPlane2->mParentTriangle.mSensor,pPlane1->mParentTriangle.mSensor)')
 $out+=$method+[Environment]::NewLine
}
Set-Content "$n\src\generated\BinderAccessKernel.cpp" $out
