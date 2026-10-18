$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\Overwrite.cpp" -Raw
$out='#include <JSystem/JParticle/JPAFieldBlock.hpp>'+"`n"+'#include <JSystem/JParticle/JPAEmitter.hpp>'+"`n"+'#include <JSystem/JParticle/JPAParticle.hpp>'+"`n"+'#include <Game/Util/MathUtil.hpp>'+"`n"
foreach($signature in @('void JPAFieldAir::prepare(','void JPAFieldVortex::prepare(','void JPAFieldVortex::calc(','void JPAFieldConvection::prepare(','void JPAFieldSpin::prepare(')){
$b=$s.IndexOf($signature);if($b -lt 0){throw "Missing original field anchor: $signature"}
$o=$s.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($s[$e] -eq '{'){$d++};if($s[$e] -eq '}'){$d--};$e++}
$out+=$s.Substring($b,$e-$b)+"`n"
}
Set-Content "$n\src\generated\JPAFieldOverrideKernel.cpp" $out
