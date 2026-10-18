$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\MapObj\AssemblyBlock.cpp" -Raw
if(!$s.Contains('MR::getRandom(0l, 2l)')){throw 'Missing original random call'}
Set-Content "$n\src\generated\AssemblyBlock.cpp" ($s.Replace('MR::getRandom(0l, 2l)','MR::getRandom(static_cast<s32>(0), static_cast<s32>(2))'))
