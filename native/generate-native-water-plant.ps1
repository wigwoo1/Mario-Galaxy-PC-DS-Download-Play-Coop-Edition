$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Map\WaterPlant.cpp" -Raw
foreach($letter in @('A','B','C','D')) {$a='"WaterPlant'+$letter+'.bti"), nullptr)';if(!$s.Contains($a)){throw 'Texture anchor missing'};$s=$s.Replace($a,'"WaterPlant'+$letter+'.bti"), static_cast<u8>(0))')}
Set-Content "$n\src\generated\WaterPlant.cpp" $s
