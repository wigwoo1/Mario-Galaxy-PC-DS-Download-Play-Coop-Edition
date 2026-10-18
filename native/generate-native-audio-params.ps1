$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\AudioLib\AudParams.cpp" -Raw
$s=$s.Replace('const int aramAddr = nullptr;','const int aramAddr = 0;')
Set-Content "$n\src\generated\AudParams.cpp" $s
