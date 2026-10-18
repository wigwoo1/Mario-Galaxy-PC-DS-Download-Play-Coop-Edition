$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content -LiteralPath "$p\Petari\src\Game\AudioLib\AudEffector.cpp" -Raw
$s="#include <Native/HeapPlacement.hpp>`r`n"+$s
Set-Content -LiteralPath "$n\src\generated\AudEffector.cpp" -Value $s
