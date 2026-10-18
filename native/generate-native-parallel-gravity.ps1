$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=[IO.File]::ReadAllText("$p\Petari\src\Game\Gravity\ParallelGravity.cpp")
$a='    positionOnWorldPlane.killElement2(rPosition - mWorldPlanePosition, mWorldPlaneUpVec);'
if(!$s.Contains($a)){throw 'Parallel projection anchor'}
$s=$s.Replace($a,'    { TVec3f relative = rPosition - mWorldPlanePosition; positionOnWorldPlane = relative + mWorldPlaneUpVec * (-mWorldPlaneUpVec.dot(relative)); }')
Set-Content "$n\src\generated\ParallelGravity.cpp" $s
