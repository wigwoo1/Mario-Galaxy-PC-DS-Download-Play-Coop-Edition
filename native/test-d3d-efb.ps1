$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\d3d_efb_tests.cpp" "$n\src\gx_depth_peek_port.cpp" "$n\src\gx_efb_readback_port.cpp" "$n\src\gx_draw_sync_completion_port.cpp" -ld3d11 -ldxgi -o "$n\build\d3d-efb-tests.exe" *> "$n\build\d3d-efb-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\d3d-efb-tests.log" -Tail 15;exit 1}
& "$n\build\d3d-efb-tests.exe"
exit $LASTEXITCODE




