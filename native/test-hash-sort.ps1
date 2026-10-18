$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include" "-I$p\Petari\include" -include revolution/types.h "$n\src\hash_sort_standalone_tests.cpp" "$n\src\hash_sort_table_port.cpp" "$n\src\sensor_hash_port.cpp" -o "$n\build\hash-sort-tests.exe" *> "$n\build\hash-sort-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\hash-sort-tests.log" -Tail 20;exit 1}
& "$n\build\hash-sort-tests.exe"
exit $LASTEXITCODE
