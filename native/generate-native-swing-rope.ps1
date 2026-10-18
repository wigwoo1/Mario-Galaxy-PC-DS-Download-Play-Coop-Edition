$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Ride\SwingRope.cpp" -Raw
$s=$s.Replace('MR::startBckPlayer("SwingRopeSlideDownStart", 0L);','MR::startBckPlayer("SwingRopeSlideDownStart", static_cast<s32>(0));').Replace('MR::startBckPlayer("SwingRopeSpin", 0L);','MR::startBckPlayer("SwingRopeSpin", static_cast<s32>(0));')
Set-Content "$n\src\generated\SwingRope.cpp" $s
