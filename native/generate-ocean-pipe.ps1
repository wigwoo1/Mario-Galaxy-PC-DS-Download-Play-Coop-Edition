$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\include\Game\Map\OceanRingPipe.hpp" -Raw
$anchor='return PI_180 * f;'
if(!$s.Contains($anchor)){throw 'Original ocean pipe angle anchor missing'}
$s=$s.Replace($anchor,'return (3.14159265358979323846f / 180.0f) * f;')
New-Item -ItemType Directory -Force "$n\include\Game\Map" | Out-Null
Set-Content "$n\include\Game\Map\OceanRingPipe.hpp" $s

