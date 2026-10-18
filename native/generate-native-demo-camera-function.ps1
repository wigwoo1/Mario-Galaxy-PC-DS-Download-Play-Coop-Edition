$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Demo\DemoCameraFunction.cpp" -Raw
$s=$s.Replace('return nullptr;', 'return false;')
$s|Set-Content "$n\src\generated\DemoCameraFunction.cpp"
