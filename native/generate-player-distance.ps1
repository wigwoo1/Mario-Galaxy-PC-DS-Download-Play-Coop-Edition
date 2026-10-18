$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\ActorMovementUtil.cpp" -Raw
$body=''
foreach($marker in @('    bool isNearPlayer(const TVec3f& rPos, f32 distance)', '    bool isNearPlayer(const LiveActor* pActor, f32 distance)', '    f32 calcDistanceToPlayer(const LiveActor*','    bool isNear(const HitSensor*','    bool isNear(const LiveActor* pActorA','    bool isNear(const LiveActor* pActor,','    bool isNearPlayerAnyTime(const LiveActor*')){
 $b=$s.IndexOf($marker);if($b -lt 0){throw "Missing original proximity method $marker"}
 $o=$s.IndexOf('{',$b);$d=1;$e=$o+1
 while($d){if($s[$e] -eq '{'){$d++}elseif($s[$e] -eq '}'){$d--};$e++}
 $body+=$s.Substring($b,$e-$b)+[Environment]::NewLine
}
Set-Content "$n\src\generated\PlayerDistanceKernel.cpp" ('#include <Game/Util/ActorMovementUtil.hpp>'+[Environment]::NewLine+'#include <Game/LiveActor/LiveActor.hpp>'+[Environment]::NewLine+'#include <Game/LiveActor/HitSensor.hpp>'+[Environment]::NewLine+'#include <Game/Scene/SceneObjHolder.hpp>'+[Environment]::NewLine+'#include <Game/Util/PlayerUtil.hpp>'+[Environment]::NewLine+'#include <Game/Util/ObjUtil.hpp>'+[Environment]::NewLine+'#include <Game/Util/MathUtil.hpp>'+[Environment]::NewLine+'namespace MR {'+$body+'}')


