$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\SoundUtil.cpp" -Raw
function ExtractSound([string]$signature){
 $start=$s.IndexOf($signature);if($start -lt 0){throw "Missing original function: $signature"}
 $open=$s.IndexOf('{',$start);$depth=1;$end=$open+1
 while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++}
 $s.Substring($start,$end-$start)
}
$body=@('#include <Game/Util/SoundUtil.hpp>','#include <Game/Util/LiveActorUtil.hpp>','#include <Game/Util/ModelUtil.hpp>','#include <Game/Util/MapUtil.hpp>','#include <Game/LiveActor/LiveActor.hpp>','#include <Game/LiveActor/Binder.hpp>','#include <Game/LiveActor/ModelManager.hpp>','#include <Game/System/ResourceHolder.hpp>','#include <Game/AudioLib/AudAnmSoundObject.hpp>','namespace MR {')
foreach($sig in @('    void startBas(','    void actorSoundMovement(','    s32 getMapSoundCodeFoot(')){$body+=(ExtractSound $sig)}
$body+='}'
Set-Content "$n\src\generated\ActorAnimationSoundKernel.cpp" ($body -join [Environment]::NewLine)
