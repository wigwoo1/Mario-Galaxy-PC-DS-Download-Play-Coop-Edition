$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& "$n\test-original-scene-registry.ps1";if($LASTEXITCODE){exit 1}
& "$n\test-original-kcl-server.ps1";if($LASTEXITCODE){exit 1}
$kernel="#include <Game/Map/CollisionCategorizedKeeper.hpp>`n#include <Game/Map/CollisionDirector.hpp>`n#include <Game/Map/CollisionParts.hpp>`n#include <Game/Util/CollisionPartsFilter.hpp>`n#include <Game/Util/MathUtil.hpp>`n#include <Game/Util/SceneUtil.hpp>`n#include <Game/Scene/SceneObjHolder.hpp>`n"
foreach($spec in @(@('Map\CollisionCategorizedKeeper.cpp',@('CollisionCategorizedKeeper::CollisionCategorizedKeeper(','CollisionCategorizedKeeper::~CollisionCategorizedKeeper()','void CollisionCategorizedKeeper::movement()','void CollisionCategorizedKeeper::addToZone','void CollisionCategorizedKeeper::removeFromZone','void CollisionCategorizedKeeper::addToGlobal','void CollisionCategorizedKeeper::removeFromGlobal','s32 CollisionCategorizedKeeper::checkStrikeBall(','bool CollisionCategorizedKeeper::isSphereOverlappingWithBox','HitInfo* CollisionCategorizedKeeper::getStrikeInfo','CollisionZone* CollisionCategorizedKeeper::getZone','void CollisionZone::calcMinMaxAndRadiusIfMoveOuter')), @('Scene\SceneObjHolder.cpp',@('SceneObjHolder::SceneObjHolder()','NameObj* SceneObjHolder::create(','NameObj* SceneObjHolder::getObj(','bool SceneObjHolder::isExist(')))){
$text=[IO.File]::ReadAllText("$p\Petari\src\Game\$($spec[0])")
foreach($signature in $spec[1]){$b=$text.IndexOf($signature);if($b -lt 0){throw "Missing $signature"};$o=$text.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($text[$e] -eq '{'){$d++};if($text[$e] -eq '}'){$d--};$e++};$kernel+=$text.Substring($b,$e-$b)+"`n"}
}
$text=[IO.File]::ReadAllText("$p\Petari\src\Game\Util\MathUtil.cpp");$b=$text.IndexOf('bool isInRange(f32');$o=$text.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($text[$e] -eq '{'){$d++};if($text[$e] -eq '}'){$d--};$e++};$kernel+="namespace MR {`n"+$text.Substring($b,$e-$b)+"`n}`n"
function ExtractOriginal($relative,$signature){
$text=[IO.File]::ReadAllText("$p\Petari\src\Game\$relative");$b=$text.IndexOf($signature);if($b -lt 0){throw "Missing $signature"};$o=$text.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($text[$e] -eq '{'){$d++};if($text[$e] -eq '}'){$d--};$e++};return $text.Substring($b,$e-$b)+"`n"
}
$kernel+="#include <Game/LiveActor/Binder.hpp>`n#include <Game/Util/MapUtil.hpp>`nnamespace { const f32 hOverlapAddValue=1.2f; }`n"
foreach($signature in @('const TVec3f Binder::bind(','void Binder::moveAlongHittedPlanes','u32 Binder::findBindedPos','bool Binder::moveWithCollisionParts','u32 Binder::storeCurrentHitInfo')){$kernel+=ExtractOriginal 'LiveActor\Binder.cpp' $signature}
foreach($signature in @('bool Triangle::isHostMoved','void Triangle::calcForceMovePower')){$kernel+=ExtractOriginal 'Map\HitInfo.cpp' $signature}
$kernel+=ExtractOriginal 'Map\CollisionParts.cpp' 'void CollisionParts::calcForceMovePower'
foreach($signature in @('void CollisionParts::resetAllMtx(const TPos3f&','void CollisionParts::resetAllMtx()','void CollisionParts::forceResetAllMtxAndSetUpdateMtxOneTime','void CollisionParts::addToBelongZone','void CollisionParts::removeFromBelongZone','void CollisionParts::resetAllMtxPrivate','void CollisionParts::updateBoundingSphereRange(TVec3f')){$kernel+=ExtractOriginal 'Map\CollisionParts.cpp' $signature}
$kernel+="#include <Game/Camera/CameraPolygonCodeUtil.hpp>`n#include <Game/Map/KCollision.hpp>`n#include <Game/Map/CollisionCode.hpp>`n"
$kernel+=ExtractOriginal 'Map\KCollision.cpp' 'bool KCollisionServer::calcFarthestVertexDistance()'
$kernel+=ExtractOriginal 'Map\KCollision.cpp' 'bool KCollisionServer::isNearParallelNormal'
$kernel+="namespace MR {`n"+(ExtractOriginal 'Util\MathUtil.cpp' 'bool normalizeOrZero(TVec3f*')+"}`n"
$kernel+="namespace {`n"+(ExtractOriginal 'Util\MapUtil.cpp' 'u32 getStrikeInfoNumCategory(s32 category) {')+"}`nnamespace Collision {`n"
foreach($signature in @('s32 checkStrikeBallToMap(const TVec3f&','s32 checkStrikeBallToMapWithMovingReaction','const HitInfo* getStrikeInfoMap','u32 getStrikeInfoNumMap()')){$kernel+=ExtractOriginal 'Util\MapUtil.cpp' $signature}
$kernel+="}`n"
[IO.File]::WriteAllText("$n\src\generated\CollisionSceneKernel.cpp",$kernel)
$line=Get-Content "$n\test-original-kcl-server.ps1" | Where-Object {$_ -like '& *zig.exe*'}
$line=$line.Replace('c++ -std','c++ -DNATIVE_COLLISION_SCENE -std').Replace(' -o ', ' "$n\src\native_collision_scene.cpp" "$n\src\generated\CollisionSceneKernel.cpp" "$p\Petari\src\Game\Map\CollisionDirector.cpp" "$p\Petari\src\Game\NameObj\NameObj.cpp" "$p\Petari\src\Game\NameObj\NameObjRegister.cpp" "$n\src\generated\NameObjHolder.cpp" "$n\src\generated\SceneMovementWrappers.cpp" "$n\src\generated\CollisionCodeMetadata.cpp" "$n\src\sensor_hash_port.cpp" -o ').Replace('original-kcl-server-tests.exe','original-collision-scene-tests.exe').Replace('original-kcl-server-tests.log','original-collision-scene-tests.log')
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$n\build\original-collision-scene-tests.log" -Tail 30;exit 1}
foreach($asset in @('broadbeanplanet','broadbeanplanetc')){& "$n\build\original-collision-scene-tests.exe" "$n\build\sample-assets\$asset.kcl" "$n\build\sample-assets\$asset.pa";if($LASTEXITCODE){exit 1}}

