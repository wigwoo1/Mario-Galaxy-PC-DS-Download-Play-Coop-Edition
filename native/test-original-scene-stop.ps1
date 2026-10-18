$ErrorActionPreference="Stop"
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" "$n\src\original_scene_stop_tests.cpp" "$p\Petari\src\Game\NameObj\NameObj.cpp" "$p\Petari\src\Game\NameObj\NameObjRegister.cpp" "$n\src\generated\NameObjHolder.cpp" "$n\src\generated\SceneMovementWrappers.cpp" "$n\src\sensor_hash_port.cpp" "$p\Petari\src\Game\NameObj\NameObjGroup.cpp" "$p\Petari\src\Game\Scene\StopSceneController.cpp" -o "$n\build\original-scene-stop-tests.exe" *> "$n\build\original-scene-stop-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\original-scene-stop-tests.log" -Tail 25;exit 1}
& "$n\build\original-scene-stop-tests.exe"
exit $LASTEXITCODE
