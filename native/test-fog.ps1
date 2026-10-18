$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\fog_tests.cpp" "$n\src\generated\FogKernel.cpp" "$n\src\gd_storage.cpp" "$n\src\gpu_resources.cpp" -o "$n\build\fog-tests.exe" *> "$n\build\fog-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\fog-tests.log" -Tail 20;exit 1}
& "$n\build\fog-tests.exe"
exit $LASTEXITCODE





