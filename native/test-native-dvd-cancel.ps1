$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$out = "$n\build\native-dvd-cancel-tests.exe"
$env:ZIG_GLOBAL_CACHE_DIR = "$n\build\native-dvd-cancel-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -Wno-nullability-completeness -target x86_64-windows-gnu `
    -include revolution/types.h "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\RVL_SDK\include" `
    "$n\src\native_dvd_port.cpp" "$n\src\native_dvd_cancel_tests.cpp" -lbcrypt -o $out *> "$n\build\native-dvd-cancel-tests.log"
if ($LASTEXITCODE) { Get-Content "$n\build\native-dvd-cancel-tests.log" -Tail 20; exit 1 }
& $out 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs'
exit $LASTEXITCODE
