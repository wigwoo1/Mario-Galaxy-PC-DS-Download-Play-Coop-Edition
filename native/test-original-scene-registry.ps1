$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$n\probe-collision-runtime.ps1"
if($LASTEXITCODE){exit 1}
# Extract original NameObj overloads; the LiveActor/LayoutActor scene wrappers stay unlinked.
$text=[IO.File]::ReadAllText("$p\Petari\src\Game\Util\ObjUtil.cpp")
$kernel="#include <Game/NameObj/NameObj.hpp>`n"
foreach($signature in @('void MR::requestMovementOn(NameObj*','void MR::requestMovementOff(NameObj*')){
$b=$text.IndexOf($signature);if($b -lt 0){throw 'movement wrapper anchor'};$o=$text.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($text[$e] -eq '{'){$d++};if($text[$e] -eq '}'){$d--};$e++};$kernel+=$text.Substring($b,$e-$b)+"`n"
}
$kernel=$kernel.Replace('#include <Game/NameObj/NameObj.hpp>','#include <Game/NameObj/NameObj.hpp>'+"`n"+'#include <Game/Util/ObjUtil.hpp>')
[IO.File]::WriteAllText("$n\src\generated\SceneMovementWrappers.cpp",$kernel)
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" "$n\src\original_scene_registry_tests.cpp" "$p\Petari\src\Game\NameObj\NameObj.cpp" "$p\Petari\src\Game\NameObj\NameObjRegister.cpp" "$n\src\generated\NameObjHolder.cpp" "$n\src\generated\SceneMovementWrappers.cpp" "$n\src\sensor_hash_port.cpp" -o "$n\build\original-scene-registry-tests.exe" *> "$n\build\original-scene-registry-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\original-scene-registry-tests.log" -Tail 25;exit 1}
& "$n\build\original-scene-registry-tests.exe"
exit $LASTEXITCODE