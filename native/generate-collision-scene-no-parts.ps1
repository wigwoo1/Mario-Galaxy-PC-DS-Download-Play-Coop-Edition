$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$source=Join-Path $n 'src\generated\CollisionSceneKernel.cpp'
$output=Join-Path $n 'src\generated\CollisionSceneKernelNoParts.cpp'
$text=[System.IO.File]::ReadAllText($source)
$expected=@{
    calcForceMovePower=1
    resetAllMtx=2
    forceResetAllMtxAndSetUpdateMtxOneTime=1
    addToBelongZone=1
    removeFromBelongZone=1
    resetAllMtxPrivate=1
    updateBoundingSphereRange=1
}
foreach($name in $expected.Keys){
    $pattern='(?ms)^[^\r\n]*\bCollisionParts::'+[regex]::Escape($name)+'\s*\([\s\S]*?\)\s*(?:const\s*)?\{[\s\S]*?^\}'
    $count=[regex]::Matches($text,$pattern).Count
    if($count -ne $expected[$name]){throw "CollisionSceneKernel expected $($expected[$name]) CollisionParts::$name definitions, found $count"}
    $text=[regex]::Replace($text,$pattern,'')
}
if($text -match 'CollisionParts::(calcForceMovePower|resetAllMtx|forceResetAllMtxAndSetUpdateMtxOneTime|addToBelongZone|removeFromBelongZone|resetAllMtxPrivate|updateBoundingSphereRange)\s*\('){throw 'CollisionSceneKernel duplicate CollisionParts methods remain'}
[System.IO.File]::WriteAllText($output,$text,[System.Text.UTF8Encoding]::new($false))
