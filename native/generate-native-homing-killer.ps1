$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Enemy\HomingKiller.cpp" -Raw
if([regex]::Matches($s,'\.copy\(\)').Count -ne 4){throw 'Unexpected original vector copy sites'}
$s=$s.Replace('mUp.copy()', 'TVec3f(mUp)').Replace('mFront.copy()', 'TVec3f(mFront)')
Set-Content "$n\src\generated\HomingKiller.cpp" $s
