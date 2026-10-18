$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
function Extract-Body($s,$marker){
 $b=$s.IndexOf($marker);if($b -lt 0){throw "Missing original $marker"};$o=$s.IndexOf('{',$b);$e=$o+1;$d=1
 while($d){if($s[$e] -eq '{'){$d++}elseif($s[$e] -eq '}'){$d--};$e++}
 return $s.Substring($b,$e-$b)
}
$s=Get-Content "$p\Petari\src\Game\Util\ActorSensorUtil.cpp" -Raw
$out='#include <Game/LiveActor/HitSensor.hpp>'+[Environment]::NewLine+'#include <Game/Util/ActorSensorUtil.hpp>'+[Environment]::NewLine+'#include <Game/Scene/SceneObjHolder.hpp>'+[Environment]::NewLine+'namespace MR {'
foreach($name in @('isSensorPlayer','isSensorRide','isSensorMapObj','isSensorAutoRush','isSensorRush')){$out+=Extract-Body $s "bool $name("}
$out+='}'+[Environment]::NewLine+'#include <Game/NameObj/NameObj.hpp>'+[Environment]::NewLine
$scene=Get-Content "$p\Petari\src\Game\Scene\SceneObjHolder.cpp" -Raw
$out+=Extract-Body $scene 'SceneObjHolder::SceneObjHolder()'
$out+=Extract-Body $scene 'NameObj* SceneObjHolder::getObj('
$out+=Extract-Body $scene 'NameObj* SceneObjHolder::create('
$nameObj=Get-Content "$p\Petari\src\Game\NameObj\NameObj.cpp" -Raw
$out+=Extract-Body $nameObj 'void NameObj::initWithoutIter('
Set-Content "$n\src\generated\SensorRoutingFixture.cpp" $out
