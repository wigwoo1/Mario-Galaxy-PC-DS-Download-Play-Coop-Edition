$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Map\SphereSelector.cpp" -Raw
$a='_A8.squareDist(MR::getStarPointerScreenPositionOrEdge(0))'
if(!$s.Contains($a)){throw 'Missing original distance call'}
$s=$s.Replace($a,'([](const TVec2f& a, const TVec2f& b) { return (a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y); })(_A8, MR::getStarPointerScreenPositionOrEdge(0))')
$s=$s.Replace('MR::getRandom(0L, 4L)','MR::getRandom(static_cast<s32>(0), static_cast<s32>(4))')
Set-Content "$n\src\generated\SphereSelector.cpp" $s
