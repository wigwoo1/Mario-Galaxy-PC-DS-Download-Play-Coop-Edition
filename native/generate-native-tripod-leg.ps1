$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Boss\TripodBossLeg.cpp" -Raw
if(!$s.Contains('_234.orthogonalize2(hitResult._18)')){throw 'Missing original leg projection anchor'}
Set-Content "$n\src\generated\TripodBossLeg.cpp" $s.Replace('_234.orthogonalize2(hitResult._18)', '_234.orthogonalize(hitResult._18)')
