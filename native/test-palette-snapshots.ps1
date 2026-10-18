$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include" "$n\src\palette_snapshot_tests.cpp" "$n\src\gpu_resources.cpp" -lpsapi -o "$n\build\palette-snapshot-tests.exe" *> "$n\build\palette-snapshot-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\palette-snapshot-tests.log" -Tail 15;exit 1}
& "$n\build\palette-snapshot-tests.exe"
exit $LASTEXITCODE
