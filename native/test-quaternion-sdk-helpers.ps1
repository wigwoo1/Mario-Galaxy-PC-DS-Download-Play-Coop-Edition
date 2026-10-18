$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$p=Split-Path $n -Parent
& "$n\generate-quaternion-sdk-helpers.ps1"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" -include revolution/types.h "$n\src\quaternion_sdk_helpers_tests.cpp" "$n\src\generated\QuaternionSdkHelpers.cpp" -o "$n\build\quaternion-sdk-helpers-tests.exe" *> "$n\build\quaternion-sdk-helpers-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\quaternion-sdk-helpers-tests.log" -Tail 20;exit 1}
& "$n\build\quaternion-sdk-helpers-tests.exe"
if($LASTEXITCODE){exit $LASTEXITCODE}
Write-Output 'PASS original C_QUATMtx identity/half-turn and C_QUATSlerp midpoint/shortest-path checks'
