$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Boss\TripodBossMovableArea.cpp" -Raw
if(!$s.Contains('v15.killElement2(v16, mBaseAxis)') -or !$s.Contains('v10.orthogonalize2(mBaseAxis)')){throw 'Original landing helper anchors missing'}
$s=$s.Replace('v15.killElement2(v16, mBaseAxis)','v15.killElement(v16, mBaseAxis)').Replace('v10.orthogonalize2(mBaseAxis)','v10.orthogonalize(mBaseAxis)')
Set-Content "$n\src\generated\TripodBossMovableArea.cpp" $s
