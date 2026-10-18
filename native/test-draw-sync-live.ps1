$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$n\generate-native-draw-sync.ps1"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\draw_sync_live_tests.cpp" "$n\src\os_mutex_port.cpp" "$n\src\native_thread_identity_port.cpp" "$n\src\os_thread_port.cpp" "$n\src\os_message_port.cpp" -o "$n\build\draw-sync-live-tests.exe" *> "$n\build\draw-sync-live-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\draw-sync-live-tests.log" -Tail 20;exit 1}
& "$n\build\draw-sync-live-tests.exe"
exit $LASTEXITCODE
