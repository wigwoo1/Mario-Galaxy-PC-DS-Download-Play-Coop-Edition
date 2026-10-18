$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
function Extract-Method([string]$source,[string]$signature){
 $begin=$source.IndexOf($signature)
 if($begin-lt 0-or $source.IndexOf($signature,$begin+1)-ge 0){throw "Missing or ambiguous method: $signature"}
 $open=$source.IndexOf('{',$begin);$end=$open+1;$depth=1
 while($depth-and $end-lt $source.Length){if($source[$end]-eq '{'){$depth++};if($source[$end]-eq '}'){$depth--};++$end}
 if($depth){throw "Unbalanced method: $signature"}
 return $source.Substring($begin,$end-$begin)
}
$creator=Get-Content -LiteralPath (Join-Path $p 'Petari\src\Game\Camera\GameCameraCreator.cpp') -Raw
$creatorMethods=foreach($signature in @('GameCameraCreator::GameCameraCreator(','void GameCameraCreator::initCameraCodeCollection(','void GameCameraCreator::registerCameraCode(','void GameCameraCreator::termCameraCodeCollection(')){Extract-Method $creator $signature}
$adapter=Get-Content -LiteralPath (Join-Path $n 'src\native_collision_scene.cpp') -Raw
$dispatch=foreach($signature in @('void initCameraCodeCollection(','void registerCameraCode(','void termCameraCodeCollection(')){Extract-Method $adapter $signature}
$abort=Extract-Method $adapter 'void abortCollisionCameraCollection('
$fixture=Get-Content -LiteralPath (Join-Path $n 'src\camera_collection_dispatch_tests.cpp') -Raw
$fixture=$fixture.Replace('// ACTUAL_CREATOR_METHODS',($creatorMethods -join "`n"))
$fixture=$fixture.Replace('// ACTUAL_DISPATCH_METHODS',("namespace MR {`n"+($dispatch -join "`n")+"`n}`nnamespace Native {`n"+$abort+"`n}"))
$out=Join-Path $n 'build\camera-collection-dispatch-test';New-Item -ItemType Directory -Force -Path $out | Out-Null
$cpp=Join-Path $out 'camera-collection-dispatch.cpp';$exe=Join-Path $out 'camera-collection-dispatch.exe'
[IO.File]::WriteAllText($cpp,$fixture)
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
& (Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ -std=c++17 -O2 -UNDEBUG -DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER -target x86_64-windows-gnu ('-I'+(Join-Path $n 'include-runtime')) ('-I'+(Join-Path $n 'include')) ('-I'+(Join-Path $p 'Petari\include')) ('-I'+(Join-Path $p 'Petari\libs\RVL_SDK\include')) -include revolution/types.h $cpp -o $exe *> (Join-Path $out 'build.log')
if($LASTEXITCODE){Get-Content -LiteralPath (Join-Path $out 'build.log') -Tail 20;exit $LASTEXITCODE}
& $exe
exit $LASTEXITCODE
