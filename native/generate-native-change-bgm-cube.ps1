$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\AreaObj\ChangeBgmCube.cpp" -Raw
$s=$s.Replace('case 0:','case 0: {').Replace('case 1:','} case 1: {').Replace('case 2:','} case 2:')
Set-Content "$n\src\generated\ChangeBgmCube.cpp" $s
