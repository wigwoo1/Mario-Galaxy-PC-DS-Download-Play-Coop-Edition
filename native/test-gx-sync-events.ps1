$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include" "$n\src\gx_sync_event_tests.cpp" "$n\src\gpu_resources.cpp" -o "$n\build\gx-sync-event-tests.exe" *> "$n\build\gx-sync-event-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\gx-sync-event-tests.log" -Tail 15;exit 1}
& "$n\build\gx-sync-event-tests.exe"
exit $LASTEXITCODE
