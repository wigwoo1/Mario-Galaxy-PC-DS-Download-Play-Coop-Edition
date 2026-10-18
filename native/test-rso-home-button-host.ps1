$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out="$n\build\rso-home-button-host-test"
New-Item -ItemType Directory -Path $out -Force|Out-Null
$env:ZIG_GLOBAL_CACHE_DIR="$n\build\rso-home-button-host-test-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -Wno-nullability-completeness -target x86_64-windows-gnu `
    "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\RVL_SDK\include" `
    -include revolution/types.h "$n\src\rso_home_button_host.cpp" "$n\src\rso_home_button_host_tests.cpp" `
    -o "$out\test-rso-home-button-host.exe"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$out\test-rso-home-button-host.exe"
if($LASTEXITCODE){exit $LASTEXITCODE}
Write-Output 'PASS: the host does not activate the unsupported Wii Home overlay and all RSO wrapper calls are safe.'
