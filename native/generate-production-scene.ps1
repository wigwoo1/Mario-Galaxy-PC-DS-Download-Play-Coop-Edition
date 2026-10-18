$ErrorActionPreference='Stop'
$n=$PSScriptRoot
function Remove-Definition([string]$source,[string]$signature){
 $start=$source.IndexOf($signature);if($start -lt 0){throw "Missing definition $signature"}
 $open=$source.IndexOf('{',$start);$depth=1;$end=$open+1
 while($depth -gt 0 -and $end -lt $source.Length){if($source[$end] -eq '{'){$depth++};if($source[$end] -eq '}'){$depth--};$end++}
 if($depth){throw "Unbalanced definition $signature"}
 return $source.Remove($start,$end-$start)
}
$kernel=Get-Content "$n\src\generated\CollisionSceneKernel.cpp" -Raw
foreach($signature in @('SceneObjHolder::SceneObjHolder()','NameObj* SceneObjHolder::create(int id)','NameObj* SceneObjHolder::getObj(int id) const','bool SceneObjHolder::isExist(int id) const')){
 $kernel=Remove-Definition $kernel $signature
}
Set-Content "$n\src\generated\ProductionCollisionKernel.cpp" $kernel
$scene=Get-Content "$n\src\native_collision_scene.cpp" -Raw
$scene=Remove-Definition $scene 'NameObj* SceneObjHolder::newEachObj(int id)'
$scene=Remove-Definition $scene 'SceneObjHolder* getSceneObjHolder()'
$scene=Remove-Definition $scene 'void connectToScene(NameObj* o,int movement,int animation,int buffer,int draw)'
foreach($signature in @('void initCameraCodeCollection(const char* name,s32 zone)','void registerCameraCode(u32 code)','void termCameraCodeCollection()')){
 $scene=Remove-Definition $scene $signature
}
$scene=$scene.Replace('#include "native_collision_scene.hpp"','#include "../native_collision_scene.hpp"')
Set-Content "$n\src\generated\ProductionCollisionScene.cpp" $scene

$p=Split-Path $n -Parent
$original=Get-Content "$p\Petari\src\Game\Util\ObjUtil.cpp" -Raw
$first=$original.IndexOf('void MR::connectToScene(NameObj*')
$last=$original.IndexOf('void MR::connectToSceneCollisionMapObj(', $first)
if($first -lt 0 -or $last -lt $first){throw 'Original scene registration range missing'}
$registration='#include "Game/NameObj/NameObjExecuteHolder.hpp"'+[Environment]::NewLine+'#include "Game/LiveActor/LiveActor.hpp"'+[Environment]::NewLine+'#include "Game/Util/ObjUtil.hpp"'+[Environment]::NewLine+$original.Substring($first,$last-$first)
Set-Content "$n\src\generated\ProductionSceneRegistration.cpp" $registration



