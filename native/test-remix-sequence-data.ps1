$ErrorActionPreference='Stop'
$native=$PSScriptRoot
$root=Split-Path $native -Parent
$tmp=Join-Path $env:TEMP 'galaxy-remix-sequence-data-tests'
New-Item -ItemType Directory -Force -Path $tmp | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $tmp 'zig-cache'
$log=Join-Path $tmp 'build.log'
$exe=Join-Path $tmp 'remix-sequence-data-tests.exe'
& "$root\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$native\include" "$native\src\remix_sequence_data_tests.cpp" "$native\src\remix_sequence_data.cpp" -o $exe *> $log
if($LASTEXITCODE){Get-Content $log -Tail 40;exit $LASTEXITCODE}
& $exe
if($LASTEXITCODE){exit $LASTEXITCODE}
Write-Output 'PASS bounded retail Remix sequence parser (big-endian and truncation cases)'
