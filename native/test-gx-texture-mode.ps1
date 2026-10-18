$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -UNDEBUG -target x86_64-windows-gnu `
 "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\RVL_SDK\include" `
 -include revolution/types.h "$n\src\gx_texture_mode_port.cpp" "$n\src\gx_texture_mode_tests.cpp" `
 -o "$n\build\gx-texture-mode-tests.exe"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\gx-texture-mode-tests.exe"
exit $LASTEXITCODE
