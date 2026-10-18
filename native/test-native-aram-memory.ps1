$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "$n\src\native_aram_memory_tests.cpp" -o "$n\build\native-aram-memory-tests.exe" *> "$n\build\native-aram-memory-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\native-aram-memory-tests.log" -Tail 10;exit 1}
& "$n\build\native-aram-memory-tests.exe"
exit $LASTEXITCODE
