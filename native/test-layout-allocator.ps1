$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu -include revolution/types.h "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" "$n\src\generated\AllocatorDispatchKernel.cpp" "$n\src\layout_allocator_tests.cpp" -o "$n\build\layout-allocator-tests.exe" *> "$n\build\layout-allocator-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\layout-allocator-tests.log" -Tail 18;exit 1}
& "$n\build\layout-allocator-tests.exe"
exit $LASTEXITCODE
