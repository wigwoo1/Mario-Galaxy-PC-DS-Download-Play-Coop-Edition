$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$n\generate-collision-runtime.ps1"
& "$n\generate-contact-classification.ps1"
& "$n\generate-binder-access.ps1"
& "$n\generate-kcollision-host-compat.ps1"
$mathSource=[IO.File]::ReadAllText("$p\Petari\src\Game\Util\MathUtil.cpp")
$boxSignature='void createBoundingBox('
$boxBegin=$mathSource.IndexOf($boxSignature)
if($boxBegin -lt 0 -or $mathSource.IndexOf($boxSignature,$boxBegin+1) -ge 0){throw 'Ambiguous original createBoundingBox method'}
$boxOpen=$mathSource.IndexOf('{',$boxBegin);$boxDepth=1;$boxEnd=$boxOpen+1
while($boxDepth){if($mathSource[$boxEnd] -eq '{'){$boxDepth++};if($mathSource[$boxEnd] -eq '}'){$boxDepth--};$boxEnd++}
$boxKernel="#include <Game/Util/MathUtil.hpp>`nnamespace MR {`n"+$mathSource.Substring($boxBegin,$boxEnd-$boxBegin)+"`n}`n"
[IO.File]::WriteAllText("$n\src\generated\CollisionBoundingBox.cpp",$boxKernel)
# Link exact generated original methods under test; full subsystem remains separately compiled.
$source=[IO.File]::ReadAllText("$n\src\generated\KCollision.cpp")
$kernel="#include `"../native_kcl_storage.hpp`"`n#include <Game/Util/MathUtil.hpp>`n"
$signatures=@('KCollisionServer::KCollisionServer(const std::vector','KCollisionServer::KCollisionServer()','void KCollisionServer::setNativeData','void KCollisionServer::updateNativeDynamicData','TVec3f KCollisionServer::getPos','void KCollisionServer::calXvec','s32 KCollisionServer::getTriangleNum','KC_PrismData* KCollisionServer::getPrismData','TVec3f* KCollisionServer::getFaceNormal','bool KCollisionServer::KCHitArrow','s32* KCollisionServer::searchBlock','KC_PrismData* KCollisionServer::checkArrow','void KCollisionServer::V3u::setUsingCast','bool KCollisionServer::KCHitSphere(','u32 KCollisionServer::checkSphere(','JMapInfoIter KCollisionServer::getAttributes','s32 KCollisionServer::toIndex','TVec3f* KCollisionServer::getEdgeNormal1','TVec3f* KCollisionServer::getEdgeNormal2','TVec3f* KCollisionServer::getEdgeNormal3','TVec3f* KCollisionServer::getNormal')
foreach($signature in $signatures){
 $begin=$source.IndexOf($signature);if($begin -lt 0 -or $source.IndexOf($signature,$begin+1) -ge 0){throw "Ambiguous method $signature"}
 $open=$source.IndexOf('{',$begin);$depth=1;$finish=$open+1
 while($depth){if($source[$finish] -eq '{'){$depth++};if($source[$finish] -eq '}'){$depth--};$finish++}
 $kernel+=$source.Substring($begin,$finish-$begin)+"`n"
}
[IO.File]::WriteAllText("$n\src\generated\OriginalKclKernel.cpp",$kernel)
$partsKernel="#include <Game/Map/CollisionParts.hpp>`n#include <Game/Map/HitInfo.hpp>`n#include <Game/Map/KCollision.hpp>`n#include <Game/Util/MathUtil.hpp>`n#include <Game/Util/TriangleFilter.hpp>`n#include <Game/LiveActor/Binder.hpp>`n#include <Game/Util/MapUtil.hpp>`n#include <Game/Map/CollisionCategorizedKeeper.hpp>`n#include <Game/Util/MtxUtil.hpp>`n"
foreach($spec in @(@('Map\CollisionParts.cpp',@('CollisionParts::CollisionParts()','u32 CollisionParts::checkStrikeLine','u32 CollisionParts::checkStrikeBall(','u32 CollisionParts::checkStrikeBallCore','void CollisionParts::calcCollidePosition','void CollisionParts::projectToPlane','void CollisionParts::updateMtx()','f32 CollisionParts::makeEqualScale','void CollisionParts::setMtx(const TPos3f&','void CollisionParts::setMtx()','void CollisionParts::updateBoundingSphereRange()','void CollisionParts::updateBoundingSphereRangePrivate')), @('Map\CollisionCategorizedKeeper.cpp',@('CollisionZone::CollisionZone(','void CollisionZone::addParts','void CollisionZone::calcMinMaxAndRadius()','void CollisionZone::addAndUpdateMinMax','void CollisionZone::eraseParts','TVec3f CollisionParts::getTrans')), @('Map\HitInfo.cpp',@('Triangle::Triangle()','void Triangle::fillData','HitInfo::HitInfo()','const TVec3f* Triangle::getFaceNormal','const TVec3f* Triangle::getNormal')), @('LiveActor\Binder.cpp',@('Binder::Binder(','void Binder::clear()','void Binder::obtainMomentFixReaction','void Binder::storeContactPlane')))){
 $text=[IO.File]::ReadAllText("$p\Petari\src\Game\$($spec[0])")
 foreach($signature in $spec[1]){$b=$text.IndexOf($signature);if($b -lt 0){throw 'parts method anchor'};$o=$text.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($text[$e] -eq '{'){$d++};if($text[$e] -eq '}'){$d--};$e++};$partsKernel+=$text.Substring($b,$e-$b)+"`n"}
}
$text=[IO.File]::ReadAllText("$p\Petari\src\Game\Util\MtxUtil.cpp")
$b=$text.IndexOf('bool isSameMtx(MtxPtr');if($b -lt 0){throw 'matrix compare anchor'};$o=$text.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($text[$e] -eq '{'){$d++};if($text[$e] -eq '}'){$d--};$e++};$partsKernel+="namespace MR {`n"+$text.Substring($b,$e-$b)+"`n}`n"
[IO.File]::WriteAllText("$n\src\generated\CollisionPartsQuery.cpp",$partsKernel)
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" "$n\src\original_kcl_server_tests.cpp" "$n\src\generated\OriginalKclKernel.cpp" "$n\src\generated\KCollisionQueriesHost.cpp" "$n\src\generated\CollisionBoundingBox.cpp" "$n\src\generated\CollisionMath.cpp" "$n\src\collision_math_host.cpp" "$n\src\generated\CollisionContactClass.cpp" "$n\src\sdk_matrix_port.cpp" "$n\src\string_equal_case_port.cpp" "$n\src\generated\CollisionPartsQuery.cpp" "$n\src\generated\BinderAccessKernel.cpp" "$n\src\generated\JMapInfo.cpp" "$p\Petari\src\JSystem\JGadget\hashcode.cpp" "$p\Petari\src\Game\Map\KCollisionPlus.cpp" -o "$n\build\original-kcl-server-tests.exe" *> "$n\build\original-kcl-server-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\original-kcl-server-tests.log" -Tail 20;exit 1}
foreach($asset in @('broadbeanplanet.kcl','broadbeanplanetc.kcl')){& "$n\build\original-kcl-server-tests.exe" "$n\build\sample-assets\$asset" "$n\build\sample-assets\$($asset.Replace('.kcl','.pa'))";if($LASTEXITCODE){exit 1}}




