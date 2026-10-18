$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\gd_storage_tests.cpp" "$n\src\gd_storage.cpp" "$n\src\gx_vertex_cache_port.cpp" "$n\src\gx_texture_cache_port.cpp" -o "$n\build\gd-storage-tests.exe" *> "$n\build\gd-storage-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\gd-storage-tests.log" -Tail 20;exit 1}
& "$n\build\gd-storage-tests.exe"
exit $LASTEXITCODE


