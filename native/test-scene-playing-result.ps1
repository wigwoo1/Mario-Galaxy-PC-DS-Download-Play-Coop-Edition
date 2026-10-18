$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -ffunction-sections "-Wl,--gc-sections" -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\scene_playing_result_tests.cpp" "$p\Petari\src\Game\Scene\ScenePlayingResult.cpp" "$p\Petari\src\Game\NameObj\NameObj.cpp" "$p\Petari\src\Game\NameObj\NameObjRegister.cpp" "$n\src\generated\NameObjHolder.cpp" "$n\src\generated\SceneMovementWrappers.cpp" "$n\src\sensor_hash_port.cpp" "$p\Petari\src\JSystem\JGadget\hashcode.cpp" -o "$n\build\scene-playing-result-tests.exe" *> "$n\build\scene-playing-result-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\scene-playing-result-tests.log" -Tail 25;exit 1}
& "$n\build\scene-playing-result-tests.exe"
exit $LASTEXITCODE


