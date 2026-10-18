$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -target x86_64-windows-gnu "$n\src\os_panic_port.cpp" "$n\src\os_version_tests.cpp" -o "$n\build\os-version-tests.exe" *> "$n\build\os-version-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\os-version-tests.log" -Tail 10;exit 1}
$output=& "$n\build\os-version-tests.exe"
if($LASTEXITCODE -or $output -ne 'native-sdk-version-fixture'){throw 'SDK version report mismatch'}
'PASS original SDK version reporting'
