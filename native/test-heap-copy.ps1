$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\heap_copy_tests.cpp" "$n\src\generated\HeapCopyKernel.cpp" -o "$n\build\heap-copy-tests.exe" *> "$n\build\heap-copy-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\heap-copy-tests.log" -Tail 10;exit 1}
& "$n\build\heap-copy-tests.exe"
exit $LASTEXITCODE
