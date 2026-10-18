$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -target x86_64-windows-gnu "$n\src\extract_native_reference_dol.cpp" -lbcrypt -o "$n\build\extract-reference-dol.exe" *> "$n\build\extract-reference-dol.log"
if($LASTEXITCODE){Get-Content "$n\build\extract-reference-dol.log" -Tail 12;exit 1}
& "$n\build\extract-reference-dol.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs' "$n\build\reference-main.dol"
exit $LASTEXITCODE
