$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$out=Join-Path $n 'build\native-dvd-reopen'
New-Item -ItemType Directory -Force -Path $out|Out-Null
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu -include revolution/types.h "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\RVL_SDK\include" "$n\src\native_dvd_port.cpp" "$n\src\native_dvd_reopen_tests.cpp" -lbcrypt -o "$out\reopen-tests.exe" *> "$out\build.txt"
if($LASTEXITCODE){Get-Content "$out\build.txt" -Tail 15;exit 1}
& "$out\reopen-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs' *> "$out\run.txt"
$code=$LASTEXITCODE;Get-Content "$out\run.txt";exit $code
