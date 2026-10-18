$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=[IO.File]::ReadAllText("$p\Petari\src\Game\Gravity\SegmentGravity.cpp")
$a='    mOppositeSideVecOrtho.killElement2(mSideVector, localAxis);'
if(!$s.Contains($a)){throw 'Segment projection anchor'}
$s=$s.Replace($a,'    mOppositeSideVecOrtho = mSideVector + localAxis * (-localAxis.dot(mSideVector));')
Set-Content "$n\src\generated\SegmentGravity.cpp" $s
