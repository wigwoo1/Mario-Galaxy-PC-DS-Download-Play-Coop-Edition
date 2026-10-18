$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_LOCAL_CACHE_DIR=Join-Path $n 'build\layout-window-material-cache'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O0 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h "$n\src\layout_window_material_tests.cpp" -lbcrypt -o "$n\build\layout-window-material-tests.exe" *> "$n\build\layout-window-material-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\layout-window-material-tests.log" -Tail 16;exit 1}
& "$n\build\layout-window-material-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs'
exit $LASTEXITCODE