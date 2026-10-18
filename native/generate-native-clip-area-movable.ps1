$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\MapObj\ClipAreaMovable.cpp" -Raw
$s=$s.Replace('JMath::gekko_ps_copy12(mBaseMtx, mtx);','mBaseMtx.set(mtx);')
Set-Content "$n\src\generated\ClipAreaMovable.cpp" $s
