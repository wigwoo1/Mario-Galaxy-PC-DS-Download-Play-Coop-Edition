$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG "$n\src\native_nand_path_tests.cpp" -o "$n\build\native-nand-path-tests.exe" *> "$n\build\native-nand-path-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\native-nand-path-tests.log" -Tail 20;exit 1}
Push-Location $n
try { & "$n\build\native-nand-path-tests.exe"; $result=$LASTEXITCODE } finally {Pop-Location}
exit $result
