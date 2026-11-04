$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& (Join-Path $n 'generate-native-audio-heap.ps1')
New-Item -ItemType Directory -Force -Path (Join-Path $n 'build\stream-pool') | Out-Null
$backend=[IO.File]::ReadAllText((Join-Path $n 'src\generated\AudioHeapKernel.cpp'))
$begin=$backend.IndexOf('JASGenericMemPool::JASGenericMemPool()')
$end=$backend.IndexOf('u32 JASKernel::sAramBase;', $begin)
if($begin-lt 0-or $end-lt $begin){throw 'Original generic pool boundaries changed'}
$kernel='#include <Native/HeapPlacement.hpp>'+"`n"+'#include <Native/AudioPoolHeapLifetime.hpp>'+"`n"+'#include <JSystem/JAudio2/JASHeapCtrl.hpp>'+"`n"+'#include <JSystem/JKernel/JKRSolidHeap.hpp>'+"`n"+$backend.Substring($begin,$end-$begin)
[IO.File]::WriteAllText((Join-Path $n 'build\stream-pool\OriginalGenericPool.cpp'),$kernel,[Text.UTF8Encoding]::new($false))
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $n 'build\zig-cache-player-display-lists'
$env:ZIG_LOCAL_CACHE_DIR=Join-Path $n 'build\zig-local-cache'
$exe=Join-Path $n 'build\audio-pool-new-tests.exe'
$log=Join-Path $n 'build\stream-pool\compile.txt'
$line=Get-Content -LiteralPath (Join-Path $n 'test-unit-heap-live.ps1')|Where-Object {$_ -like '& *zig.exe*'}
if(@($line).Count-ne 1){throw 'Heap fixture compile anchor changed'}
$line=$line.Replace('unit_heap_live_tests.cpp','audio_pool_new_tests.cpp')
$line=$line.Replace(' -o $exe',' "$n\build\stream-pool\OriginalGenericPool.cpp" -o $exe')
Invoke-Expression $line
if($LASTEXITCODE){Get-Content -LiteralPath $log -Tail 20;exit 1}
& $exe *> (Join-Path $n 'build\stream-pool\run.txt')
$result=$LASTEXITCODE
Get-Content -LiteralPath (Join-Path $n 'build\stream-pool\run.txt')
exit $result
