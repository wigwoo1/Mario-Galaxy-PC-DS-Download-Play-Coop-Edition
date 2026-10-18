$ErrorActionPreference = 'Stop'
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR = "$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++20 -O2 -UNDEBUG -target x86_64-windows-gnu `
    "-I$n\include" "$n\src\dsp_memory_view_tests.cpp" "$n\src\dsp_memory_view.cpp" `
    "$n\src\dsp_address_space.cpp" -o "$n\build\dsp-memory-view-tests.exe"
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& "$n\build\dsp-memory-view-tests.exe"
exit $LASTEXITCODE
