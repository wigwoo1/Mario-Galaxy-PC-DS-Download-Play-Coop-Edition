$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Boss\TripodBossKillerGeneraterCircle.cpp" -Raw
if([regex]::Matches($s,'0x1000000').Count -ne 2){throw 'Unexpected original collision flags'}
Set-Content "$n\src\generated\TripodBossKillerGeneraterCircle.cpp" $s.Replace('0x1000000','true')
