$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
$line=Get-Content "$n\test-unit-heap-live.ps1" | Where-Object {$_ -like '& *zig.exe*'}
$providers=' "$n\src\native_jkr_aram_port.cpp" "$n\src\generated\JKRAramHeap.cpp" "$n\src\generated\JKRAramBlock.cpp" "$n\src\generated\AramCompletionKernel.cpp" "$n\src\generated\AramCommandKernel.cpp" "$n\src\generated\JKRDecomp.cpp" "$n\src\generated\JKRThread.cpp" "$n\src\jut_console_port.cpp" "$n\src\sdk_cache_port.cpp" "$n\src\gpu_resources.cpp" '
$line=$line.Replace('src\unit_heap_live_tests.cpp','src\original_aram_command_tests.cpp').Replace(' -o ',$providers+' -o ').Replace('unit-heap-live-tests','original-aram-command-tests')
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$n\build\original-aram-command-tests.log" -Tail 15;exit 1}
& "$n\build\original-aram-command-tests.exe"
exit $LASTEXITCODE

