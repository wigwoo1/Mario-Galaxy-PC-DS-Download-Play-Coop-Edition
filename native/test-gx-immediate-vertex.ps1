$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -UNDEBUG -target x86_64-windows-gnu `
 "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\RVL_SDK\include" `
 -include revolution/types.h "$n\src\gx_immediate_vertex_port.cpp" "$n\src\gx_immediate_vertex_tests.cpp" `
 -o "$n\build\gx-immediate-vertex-tests.exe" *> "$n\build\gx-immediate-vertex-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\gx-immediate-vertex-tests.log" -Tail 15;exit 1}
& "$n\build\gx-immediate-vertex-tests.exe"
exit $LASTEXITCODE
