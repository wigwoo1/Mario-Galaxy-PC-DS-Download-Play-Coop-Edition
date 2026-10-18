$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Demo\DemoTimeKeeper.cpp" -Raw
$s=$s.Replace('inline s32 DemoTimeKeeper::getPart(', 's32 DemoTimeKeeper::getPart(')
$s|Set-Content "$n\src\generated\DemoTimeKeeper.cpp"
