$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\ghost_packet_tests.cpp" "$n\src\native_ghost_packet_port.cpp" -o "$n\build\ghost-packet-tests.exe" *> "$n\build\ghost-packet-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\ghost-packet-tests.log" -Tail 20;exit 1}
& "$n\build\ghost-packet-tests.exe"
exit $LASTEXITCODE
