$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
$compiler="$p\tools\zig-x86_64-windows-0.16.0\zig.exe"
$include=@("-I$n\include-runtime","-I$n\include","-I$p\Petari\include","-I$p\Petari\libs\JSystem\include","-I$p\Petari\libs\RVL_SDK\include","-I$p\Petari\libs\nw4r\include")
& $compiler c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu @include -include revolution/types.h "$n\src\native_nwc24_host_tests.cpp" "$n\src\native_nwc24_host.cpp" -o "$n\build\nwc24-host-tests.exe" *> "$n\build\nwc24-host-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\nwc24-host-tests.log" -Tail 25;exit 1}
& "$n\build\nwc24-host-tests.exe"
exit $LASTEXITCODE
