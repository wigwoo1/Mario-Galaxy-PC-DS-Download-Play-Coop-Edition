$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\RVL_SDK\include" "$n\src\rfl_matrix_support_tests.cpp" "$n\src\rfl_matrix_support_port.cpp" -o "$n\build\rfl-matrix-support-tests.exe" *> "$n\build\rfl-matrix-support-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\rfl-matrix-support-tests.log" -Tail 20;exit 1}
& "$n\build\rfl-matrix-support-tests.exe"
exit $LASTEXITCODE
