$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& "$n\generate-collision-runtime.ps1"
$s=[IO.File]::ReadAllText("$p\Petari\src\Game\MapObj\DynamicCollisionObj.cpp")
$s="#include `"../native_dynamic_kcl.hpp`"`n"+$s
$a='    mParts->init(posmtx, getSensor("body"), mKCLFile, nullptr, 0, true);'
if(!$s.Contains($a)){throw 'Dynamic collision creation anchor'}
$s=$s.Replace($a,'    mParts->initNative(posmtx, getSensor("body"), encodeDynamicKcl(*mKCLFile, _94, std::size_t(_94) * 4, _94), {}, 0, true);')
$a="void DynamicCollisionObj::syncCollision() {`r`n    updateCollisionHeader();`r`n    updateTriangle();"
if(!$s.Contains($a)){throw 'Dynamic collision sync anchor'}
$s=$s.Replace($a,$a+"`r`n    mParts->mServer->updateNativeDynamicData(encodeDynamicKcl(*mKCLFile, _94, std::size_t(_94) * 4, _94));")
# Emit original initNative body with bounded loading, without raw-pointer init.
$t=[IO.File]::ReadAllText("$n\src\generated\CollisionParts.cpp")
$b=$t.IndexOf('void CollisionParts::initNative(');if($b -lt 0){throw 'Native initializer anchor'}
$o=$t.IndexOf('{',$b);$depth=1;$e=$o+1
while($depth){if($t[$e] -eq '{'){$depth++};if($t[$e] -eq '}'){$depth--};$e++}
$s+="`n#include <Game/LiveActor/HitSensor.hpp>`n#include <Game/Map/CollisionDirector.hpp>`n#include <Game/Map/CollisionCategorizedKeeper.hpp>`n#include <Game/Camera/CameraPolygonCodeUtil.hpp>`n"+$t.Substring($b,$e-$b)
Set-Content "$n\src\generated\DynamicCollisionObj.cpp" $s
