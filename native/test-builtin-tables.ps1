$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$n\generate-jmap-runtime.ps1"
& "$n\generate-native-builtin-tables.ps1"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" -include "$n\include-runtime\revolution\os\OSTime.h" "$n\src\builtin_table_tests.cpp" "$n\src\generated\JMapInfo.cpp" "$n\src\generated\NativeBuiltinTables.cpp" "$n\src\generated\StringKernel.cpp" "$n\src\sensor_hash_port.cpp" "$p\Petari\src\JSystem\JGadget\hashcode.cpp" -o "$n\build\builtin-table-tests.exe" *> "$n\build\builtin-table-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\builtin-table-tests.log" -Tail 15;exit 1}
& "$n\build\builtin-table-tests.exe" "$p\Petari\src\Game\System\GalaxyID.bcsv" galaxy
if($LASTEXITCODE){exit 1}
& "$n\build\builtin-table-tests.exe" "$p\Petari\src\Game\System\StoryEvent.bcsv" story
exit $LASTEXITCODE

