$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu -include revolution/types.h "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\RVL_SDK\include" "$n\src\native_dvd_port.cpp" "$n\src\native_dvd_tests.cpp" -lbcrypt -o "$n\build\native-dvd-tests.exe" *> "$n\build\native-dvd-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\native-dvd-tests.log" -Tail 10;exit 1}
& "$n\build\native-dvd-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs'
exit $LASTEXITCODE


