$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -DGALAXY_NATIVE_SCENARIO_SOURCE_ONLY -std=c++17 -O2 -UNDEBUG -ffunction-sections "-Wl,--gc-sections" -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" "$n\src\archive_jmap_game_tests.cpp" "$n\src\generated\JMapInfo.cpp" "$n\src\generated\ScenarioParserStartupKernel.cpp" "$n\src\generated\ScenarioDataKernel.cpp" "$n\src\generated\ScenarioServicesKernel.cpp" "$p\Petari\src\Game\NameObj\NameObj.cpp" "$p\Petari\src\Game\NameObj\NameObjRegister.cpp" "$n\src\generated\NameObjHolder.cpp" "$n\src\generated\SceneMovementWrappers.cpp" "$n\src\sensor_hash_port.cpp" "$n\src\generated\GalaxyNameSortTable.cpp" "$p\Petari\src\JSystem\JGadget\hashcode.cpp" "$n\src\string_equal_case_port.cpp" -lbcrypt -o "$n\build\archive-jmap-game-tests.exe" *> "$n\build\archive-jmap-game-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\archive-jmap-game-tests.log" -Tail 20;exit 1}
& "$n\build\archive-jmap-game-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs'
exit $LASTEXITCODE













