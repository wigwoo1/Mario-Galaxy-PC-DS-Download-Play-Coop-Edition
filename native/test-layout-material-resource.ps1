$ErrorActionPreference='Stop'
$native=$PSScriptRoot;$project=Split-Path $native -Parent
$output=Join-Path $env:TEMP 'galaxy-layout-material-resource-tests'
New-Item -ItemType Directory -Force -Path $output | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $env:TEMP 'galaxy-prepresent-test-cache'
$exe=Join-Path $output 'layout-material-resource-tests.exe';$log=Join-Path $output 'build.log'
& "$project\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$native\include-runtime" "-I$native\include" "-I$project\Petari\libs\RVL_SDK\include" "-I$project\Petari\libs\nw4r\include" -include revolution/types.h "$native\src\layout_material_resource_tests.cpp" -o $exe *> $log
if($LASTEXITCODE){Get-Content -LiteralPath $log -Tail 20;exit 1}
& $exe
exit $LASTEXITCODE
