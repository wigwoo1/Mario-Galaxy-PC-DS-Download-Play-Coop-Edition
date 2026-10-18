$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\LiveActor\EffectKeeper.cpp" -Raw
$b=$s.IndexOf('void EffectKeeper::changeBck()')
if($b -lt 0){throw 'Original effect animation reset anchor'}
$o=$s.IndexOf('{',$b);$d=1;$e=$o+1
while($d){if($s[$e] -eq '{'){$d++};if($s[$e] -eq '}'){$d--};$e++}
Set-Content "$n\src\generated\EffectAnimationResetKernel.cpp" ('#include <Game/LiveActor/EffectKeeper.hpp>'+[Environment]::NewLine+'#include <Game/Effect/SyncBckEffectChecker.hpp>'+[Environment]::NewLine+$s.Substring($b,$e-$b)) -Encoding UTF8
