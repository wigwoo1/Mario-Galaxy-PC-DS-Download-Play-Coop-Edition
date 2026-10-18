$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\JAudio2\JAUInitializer.cpp" -Raw
$s=$s.Replace('inline void JAU_JASInitializer::initJASystem','void JAU_JASInitializer::initJASystem')
Set-Content "$n\src\generated\JAUInitializer.cpp" $s
