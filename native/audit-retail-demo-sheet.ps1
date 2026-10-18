$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -target x86_64-windows-gnu "$n\src\retail_demo_sheet_audit.cpp" -lbcrypt -o "$n\build\audit-retail-demo-sheet.exe" *> "$n\build\audit-retail-demo-sheet.log"
if($LASTEXITCODE){Get-Content "$n\build\audit-retail-demo-sheet.log" -Tail 12;exit 1}
& "$n\build\audit-retail-demo-sheet.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs' "$n\build\retail-demo-time.bcsv"
exit $LASTEXITCODE

