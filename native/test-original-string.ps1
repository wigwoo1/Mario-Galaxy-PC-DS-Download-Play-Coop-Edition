$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "$n\src\generated\StringKernel.cpp" "$n\src\original_string_tests.cpp" -o "$n\build\original-string-tests.exe" *> "$n\build\original-string-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\original-string-tests.log" -Tail 12;exit 1}
& "$n\build\original-string-tests.exe"
if($LASTEXITCODE){exit $LASTEXITCODE}
'PASS original case comparison and narrow/wide bounded copying'
