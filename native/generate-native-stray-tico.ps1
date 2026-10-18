$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\NPC\StrayTico.cpp" -Raw
if(!$s.Contains('vec.killElement2(rParam1 - mPosition, _C8);')){throw 'Missing original helper'}
Set-Content "$n\src\generated\StrayTico.cpp" ($s.Replace('vec.killElement2(rParam1 - mPosition, _C8);','vec.killElement(rParam1 - mPosition, _C8);'))
