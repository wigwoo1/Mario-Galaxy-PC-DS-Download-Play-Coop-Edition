$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Ride\SphereAccelSensorController.cpp" -Raw
foreach($v in @('accelXY','accelYZ')){
$a=$v+'.isZero()';if(!$s.Contains($a)){throw 'Original vector check missing'}
$s=$s.Replace($a,'('+ $v+'.x * '+$v+'.x + '+$v+'.y * '+$v+'.y <= JGeometry::TUtil<f32>::epsilon())')
}
$s='#include <JSystem/JGeometry/TUtil.hpp>'+"`r`n"+$s
Set-Content "$n\src\generated\SphereAccelSensorController.cpp" $s
