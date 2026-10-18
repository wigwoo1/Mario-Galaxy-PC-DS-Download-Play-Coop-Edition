$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\MapObj\ElectricRailHolder.cpp" -Raw
if(!$s.Contains('ARRAY_SIZE(mRailModels)')){throw 'Original fixed array anchor missing'}
Set-Content "$n\src\generated\ElectricRailHolder.cpp" $s.Replace('ARRAY_SIZE(mRailModels)','mRailModels.size()')
