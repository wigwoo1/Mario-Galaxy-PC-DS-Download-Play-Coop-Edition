$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\System\Overwrite.cpp" -Raw
$out='#include <JSystem/JParticle/JPAEmitter.hpp>'+"`n"+'#include <JSystem/JParticle/JPAParticle.hpp>'+"`n"+'#include <JSystem/JParticle/JPAResource.hpp>'+"`n"+'#include <JSystem/JParticle/JPABaseShape.hpp>'+"`n"+'#include <JSystem/JMath/JMATrigonometric.hpp>'+"`n"+'#include <Game/Util/MathUtil.hpp>'+"`n"
$b=$s.LastIndexOf('namespace {',$s.IndexOf('static void noLoadPrj('));if($b -lt 0){throw 'Original particle drawing helper namespace missing'}
$o=$s.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($s[$e] -eq '{'){$d++};if($s[$e] -eq '}'){$d--};$e++};$out+=$s.Substring($b,$e-$b)+"`n"
foreach($signature in @('void JPADrawDirection(','void JPADrawRotDirection(','void JPADrawDBillboard(','void JPADrawYBillboard(','void JPADrawRotYBillboard(','void JPADrawLine(','void JPADrawStripe(','void JPADrawStripeX(')){
$b=$s.IndexOf($signature);if($b -lt 0){throw "Original draw anchor missing: $signature"};$o=$s.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($s[$e] -eq '{'){$d++};if($s[$e] -eq '}'){$d--};$e++};$out+=$s.Substring($b,$e-$b)+"`n"
}
Set-Content "$n\src\generated\JPADrawingOverrideKernel.cpp" $out

