$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -UNDEBUG -target x86_64-windows-gnu "-I$n\include" "$n\src\camera_animation_data_tests.cpp" -o "$n\build\camera-animation-data-tests.exe"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\camera-animation-data-tests.exe"
exit $LASTEXITCODE
