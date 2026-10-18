$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\display_copy_filter_tests.cpp" "$n\src\generated\DisplayCopyFilterKernel.cpp" "$n\src\gd_storage.cpp" "$n\src\gpu_resources.cpp" -o "$n\build\display-copy-filter-tests.exe" *> "$n\build\display-copy-filter-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\display-copy-filter-tests.log" -Tail 20;exit 1}
& "$n\build\display-copy-filter-tests.exe"
exit $LASTEXITCODE




