$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out=Join-Path $env:TEMP 'smg-native-xfb-registry'
New-Item -ItemType Directory -Force -Path $out | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
$exe=Join-Path $out 'xfb-resource-registry-tests.exe'
$log=Join-Path $out 'xfb-resource-registry-tests.log'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" -include revolution/types.h "$n\src\xfb_resource_registry_tests.cpp" "$n\src\gpu_resources.cpp" -o $exe *> $log
if($LASTEXITCODE){Get-Content -LiteralPath $log -Tail 30;exit 1}
& $exe
exit $LASTEXITCODE
