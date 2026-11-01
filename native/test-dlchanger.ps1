$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $n 'build\zig-cache-player-display-lists'
$env:ZIG_LOCAL_CACHE_DIR=Join-Path $n 'build\zig-local-cache'
$exe=Join-Path $n 'build\dlchanger-tests.exe'
$log=Join-Path $n 'build\dlchanger-tests.log'
$runLog=Join-Path $n 'build\dlchanger-tests-run.log'
$line=Get-Content -LiteralPath (Join-Path $n 'test-unit-heap-live.ps1') | Where-Object {$_ -like '& *zig.exe*'}
if(@($line).Count-ne 1){throw 'Expected one real heap regression compiler command'}
$line=$line.Replace('unit_heap_live_tests.cpp','dlchanger_tests.cpp')
$providers=' "$n\src\gpu_resources.cpp" "$n\src\gd_storage.cpp" "$n\src\gx_array_port.cpp" "$n\src\sdk_cache_port.cpp" '
$line=$line.Replace(' -o $exe',$providers+' -o $exe')
Invoke-Expression $line
if($LASTEXITCODE){Get-Content -LiteralPath $log -Tail 30;exit 1}
& $exe *> $runLog
$result=$LASTEXITCODE
Get-Content -LiteralPath $runLog
exit $result