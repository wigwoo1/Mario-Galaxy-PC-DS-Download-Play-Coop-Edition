$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
$line=Get-Content "$n\test-unit-heap-live.ps1" | Where-Object {$_ -like '& *zig.exe*'}
$providers=' "$n\src\native_dvd_port.cpp" "$n\src\generated\JKRDvdFile.cpp" "$n\src\jut_console_port.cpp" '
$line=$line.Replace('src\unit_heap_live_tests.cpp','src\original_dvd_file_tests.cpp').Replace(' -o ',$providers+' -lbcrypt -o ').Replace('unit-heap-live-tests','original-dvd-file-tests')
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$n\build\original-dvd-file-tests.log" -Tail 12;exit 1}
& "$n\build\original-dvd-file-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs'
exit $LASTEXITCODE
