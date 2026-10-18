param([switch]$Original)
$ErrorActionPreference='Stop'
$native=$PSScriptRoot
$root=Split-Path $native -Parent
$sourcePath=if($Original){Join-Path $root 'Petari\src\JSystem\JKernel\JKRFileLoader.cpp'}else{Join-Path $native 'src\generated\JKRFileLoader.cpp'}
$source=Get-Content -LiteralPath $sourcePath -Raw
$begin=$source.IndexOf('void JKRFileLoader::unmount() {')
$end=$source.IndexOf('void* JKRFileLoader::getGlbResource(', $begin)
if($begin-lt 0-or $end-le $begin){throw 'Could not extract actual loader unmount method.'}
$fixture=Get-Content -LiteralPath (Join-Path $native 'src\file_loader_unmount_tests.cpp') -Raw
$fixture=$fixture.Replace('// ACTUAL_UNMOUNT_INSERTION_POINT',$source.Substring($begin,$end-$begin))
$out=Join-Path $native 'build\file-loader-unmount-test'
New-Item -ItemType Directory -Force -Path $out|Out-Null
$cpp=Join-Path $out 'file_loader_unmount_tests.cpp'
$exe=Join-Path $out 'file-loader-unmount-tests.exe'
[IO.File]::WriteAllText($cpp,$fixture,[Text.UTF8Encoding]::new($false))
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $out 'zig-cache'
$env:ZIG_LOCAL_CACHE_DIR=Join-Path $out 'local-cache'
& (Join-Path $root 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ -std=c++17 -O2 -fno-exceptions -fno-rtti -nostdlib -target x86_64-windows-gnu '-Wl,--entry,mainCRTStartup' $cpp -lkernel32 -o $exe
if($LASTEXITCODE){exit $LASTEXITCODE}
& $exe
if($LASTEXITCODE){Write-Output "FAIL loader unmount: exit $LASTEXITCODE";exit $LASTEXITCODE}
Write-Output 'PASS actual loader unmount: zero/shared references retain storage; final reference invokes the derived destructor and releases the original JKR allocation once without host delete.'
