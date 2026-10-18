$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$n\generate-native-dvd-ripper.ps1"
$line=Get-Content "$n\test-unit-heap-live.ps1" | Where-Object {$_ -like '& *zig.exe*'}
$ripper=Get-Content "$n\src\generated\JKRDvdRipperKernel.cpp" -Raw
$ripper=$ripper.Replace('DVDReadPrio(', 'nativeTestDVDReadPrio(')
$ripper="#include <revolution.h>`r`nextern `"C`" s32 nativeTestDVDReadPrio(DVDFileInfo*,void*,s32,s32,s32);`r`n"+$ripper
$ripper|Set-Content "$n\build\JKRDvdRipperFaultFixture.cpp"
& "$n\generate-audio-arc-interpreter.ps1"
$inspector=Get-Content "$n\src\retail_audio_arc_tests.cpp" -Raw
$main=$inspector.IndexOf('int main(')
if($main -lt 0){throw 'Retail audio inspector boundary missing'}
$inspector.Substring(0,$main)|Set-Content "$n\build\AudioArcInspectorFixture.hpp"
$providers=' "$n\src\native_dvd_port.cpp" "$n\src\generated\JKRDvdFile.cpp" "$n\src\jut_console_port.cpp" "$n\src\os_critical_port.cpp" "$n\build\JKRDvdRipperFaultFixture.cpp" "$n\src\generated\JKRDecoderKernel.cpp" "$n\src\generated\AudioArcInterpreterKernel.cpp" "$n\src\bank_chunk_port.cpp" '
$line=$line.Replace(' -O2 ', ' -O2 "-DIS_NOT_ALIGNED(X,N)=((X)&((N)-1))" ')
$line=$line.Replace('src\unit_heap_live_tests.cpp','src\original_dvd_ripper_tests.cpp').Replace(' -o ',$providers+' -lbcrypt -o ').Replace('unit-heap-live-tests','original-dvd-ripper-tests')
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$n\build\original-dvd-ripper-tests.log" -Tail 12;exit 1}
& "$n\build\original-dvd-ripper-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs'
exit $LASTEXITCODE







