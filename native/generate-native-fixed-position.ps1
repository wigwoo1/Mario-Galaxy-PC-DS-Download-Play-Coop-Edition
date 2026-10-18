$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\FixedPosition.cpp" -Raw
$s=$s.Replace('MR::getCsvDataVec(&trans,','MR::getCsvDataVec(reinterpret_cast<Vec*>(&trans),').Replace('MR::getCsvDataVec(&rotate,','MR::getCsvDataVec(reinterpret_cast<Vec*>(&rotate),')
Set-Content "$n\src\generated\FixedPosition.cpp" $s
