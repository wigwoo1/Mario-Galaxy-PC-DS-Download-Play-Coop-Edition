$ErrorActionPreference='Stop'
$native=$PSScriptRoot
$project=Split-Path $native -Parent
$output=Join-Path $env:TEMP 'galaxy-scene-connection-tests'
New-Item -ItemType Directory -Force -Path $output | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $output 'zig-cache'
$exe=Join-Path $output 'scene-connection-tests.exe'
$log=Join-Path $output 'build.log'
# Extract the actual adapter definition, as the existing kernel tests do, to
# avoid linking unrelated terrain/camera services into this boundary check.
$source=Get-Content (Join-Path $native 'src\native_collision_scene.cpp') -Raw
$signature='void connectToScene(NameObj* o,int movement,int animation,int buffer,int draw)'
$start=$source.IndexOf($signature)
if($start -lt 0){throw 'Native scene adapter definition missing'}
$open=$source.IndexOf('{',$start);$depth=1;$end=$open+1
while($depth -and $end -lt $source.Length){if($source[$end] -eq '{'){$depth++};if($source[$end] -eq '}'){$depth--};$end++}
if($depth){throw 'Unbalanced native scene adapter definition'}
$header=(Join-Path $native 'src\native_collision_scene.hpp').Replace('\','/')
$kernel='#include "'+$header+'"' + "`n" + '#include <Game/NameObj/NameObjExecuteHolder.hpp>' + "`n" + '#include <Game/Util/ObjUtil.hpp>' + "`n" + '#include <Game/Scene/SceneFunction.hpp>' + "`n" + '#include <algorithm>' + "`n" + '#include <stdexcept>' + "`n" + 'namespace { NativeCollisionScene* active=nullptr; NativeCollisionScene& current(){if(!active)throw std::logic_error("No active test scene");return *active;} }' + "`n" + 'namespace MR {' + "`n" + $source.Substring($start,$end-$start) + "`n}" + "`n"
$kernelPath=Join-Path $output 'scene_connection_kernel.cpp'
[IO.File]::WriteAllText($kernelPath,$kernel,[Text.UTF8Encoding]::new($false))
& "$project\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$native\include-runtime" "-I$native\include" "-I$project\Petari\include" "-I$project\Petari\libs\JSystem\include" "-I$project\Petari\libs\RVL_SDK\include" "-I$project\Petari\libs\nw4r\include" -include revolution/types.h -include "$native\include-runtime\revolution\os.h" -include "$native\include-runtime\revolution\gx\GXVert.h" -include "$native\include-runtime\revolution\os\OSTime.h" $kernelPath "$native\src\scene_connection_tests.cpp" -o $exe *> $log
if($LASTEXITCODE){Get-Content -LiteralPath $log -Tail 25;exit 1}
& $exe
exit $LASTEXITCODE
