$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include" "$n\src\tev_arithmetic_tests.cpp" -o "$n\build\tev-arithmetic-tests.exe" *> "$n\build\tev-arithmetic-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\tev-arithmetic-tests.log" -Tail 20;exit 1}
& "$n\build\tev-arithmetic-tests.exe" | Tee-Object "$n\build\tev-arithmetic-run.log"
exit $LASTEXITCODE
