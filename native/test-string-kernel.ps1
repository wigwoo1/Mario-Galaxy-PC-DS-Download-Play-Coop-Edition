$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include" "-I$p\Petari\include" -include revolution/types.h "$n\src\string_kernel_tests.cpp" "$n\src\generated\StringKernel.cpp" -o "$n\build\string-kernel-tests.exe" *> "$n\build\string-kernel-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\string-kernel-tests.log" -Tail 15;exit 1}
& "$n\build\string-kernel-tests.exe"
exit $LASTEXITCODE
