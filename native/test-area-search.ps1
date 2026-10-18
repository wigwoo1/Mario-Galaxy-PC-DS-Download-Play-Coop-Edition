$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -target x86_64-windows-gnu "-I$n\include" "$n\src\area_search_tests.cpp" -o "$n\build\area-search-tests.exe" *> "$n\build\area-search-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\area-search-tests.log" -Tail 15;exit 1}
& "$n\build\area-search-tests.exe"
exit $LASTEXITCODE
