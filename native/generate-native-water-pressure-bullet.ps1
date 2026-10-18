$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\MapObj\WaterPressureBullet.cpp" -Raw
$s=$s.Replace('MR::startBckPlayer("GCaptureBreak", 0L);','MR::startBckPlayer("GCaptureBreak", static_cast<s32>(0));').Replace('MR::startBckPlayer("Spin2nd", 0L);','MR::startBckPlayer("Spin2nd", static_cast<s32>(0));')
Set-Content "$n\src\generated\WaterPressureBullet.cpp" $s

