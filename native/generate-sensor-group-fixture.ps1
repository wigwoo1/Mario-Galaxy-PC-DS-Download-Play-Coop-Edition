$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\LiveActor\SensorHitChecker.cpp" -Raw
$b=$s.IndexOf('SensorGroup::SensorGroup(');$e=$s.IndexOf('void SensorHitChecker::init(')
if($b -lt 0 -or $e -le $b){throw 'Original SensorGroup boundaries missing'}
Set-Content "$n\src\generated\SensorGroupFixture.cpp" ('#include <Game/LiveActor/SensorHitChecker.hpp>'+[Environment]::NewLine+'#include <Game/LiveActor/HitSensor.hpp>'+[Environment]::NewLine+$s.Substring($b,$e-$b))
