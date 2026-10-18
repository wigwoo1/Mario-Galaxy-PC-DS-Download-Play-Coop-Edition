$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out=Join-Path $env:TEMP 'smg-native-d3d-xfb-present'
New-Item -ItemType Directory -Force -Path $out | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
$exe=Join-Path $out 'd3d-xfb-present-tests.exe'
$log=Join-Path $out 'd3d-xfb-present-tests.log'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" -include revolution/types.h "$n\src\d3d_xfb_present_tests.cpp" -ld3dcompiler_47 -ld3d11 -ldxgi -luser32 -o $exe *> $log
if($LASTEXITCODE){Get-Content -LiteralPath $log -Tail 40;exit 1}
& $exe
exit $LASTEXITCODE
