$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\water_info_tests.cpp" "$p\Petari\src\Game\Map\WaterInfo.cpp" -o "$n\build\water-info-tests.exe" *> "$n\build\water-info-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\water-info-tests.log" -Tail 20;exit 1}
& "$n\build\water-info-tests.exe"
exit $LASTEXITCODE
