$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" cc -std=gnu99 -O2 -c -target x86_64-windows-gnu "$n\src\host_c_compat_port.c" -o "$n\build\host-c-compat-port.obj" *> "$n\build\host-c-compat-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\host-c-compat-tests.log" -Tail 20;exit 1}
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "$n\src\host_c_compat_tests.cpp" "$n\build\host-c-compat-port.obj" -o "$n\build\host-c-compat-tests.exe" *> "$n\build\host-c-compat-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\host-c-compat-tests.log" -Tail 20;exit 1}
& "$n\build\host-c-compat-tests.exe"
exit $LASTEXITCODE
