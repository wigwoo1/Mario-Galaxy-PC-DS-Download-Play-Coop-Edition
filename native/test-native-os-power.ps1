$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
$n = $PSScriptRoot
$p = Split-Path $n -Parent
$out = "$n\build\native-os-power-tests.exe"
$env:ZIG_GLOBAL_CACHE_DIR = "$n\build\native-os-power-cache"
$previousErrorActionPreference = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
try {
    & "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -Wno-nullability-completeness -Wno-gcc-compat -Wno-nonportable-include-path -Wno-deprecated-declarations -target x86_64-windows-gnu `
        "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\RVL_SDK\include" `
        "$n\src\native_os_power_port.cpp" "$n\src\native_os_power_port_tests.cpp" -o $out *> "$n\build\native-os-power-tests.log"
    $compileExit = $LASTEXITCODE
} finally { $ErrorActionPreference = $previousErrorActionPreference }
if ($compileExit) { Get-Content "$n\build\native-os-power-tests.log" -Tail 20; exit $compileExit }
& $out
exit $LASTEXITCODE
