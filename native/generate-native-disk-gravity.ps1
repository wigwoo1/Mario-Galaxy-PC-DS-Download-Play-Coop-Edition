$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=[IO.File]::ReadAllText("$p\Petari\src\Game\Gravity\DiskGravity.cpp")
$a='    mOppositeSideVecOrtho.killElement2(mSideDirection, mLocalNormal);'
if(!$s.Contains($a)){throw 'Disk projection anchor'}
$s=$s.Replace($a,'    mOppositeSideVecOrtho = mSideDirection + mLocalNormal * (-mLocalNormal.dot(mSideDirection));')
Set-Content "$n\src\generated\DiskGravity.cpp" $s
