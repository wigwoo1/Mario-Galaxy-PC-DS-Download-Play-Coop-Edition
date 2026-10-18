param([switch]$OriginalCallbackSizes)
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out=Join-Path $n 'build\jpa-archive-test';New-Item -ItemType Directory -Force -Path $out|Out-Null
$exe=Join-Path $out 'jpa-archive-tests.exe'
$callbackSource=if($OriginalCallbackSizes){Join-Path $p 'Petari\src\JSystem\JParticle\JPAResource.cpp'}else{Join-Path $n 'src\generated\JPAResource.cpp'}
$callbackText=Get-Content -LiteralPath $callbackSource -Raw
$allocations=[regex]::Matches($callbackText,'(?m)^\s*mp\w+FuncList = \((?:EmitterFunc|ParticleFunc)\*\)JKRAllocFromHeap\([^;]+;')
if($allocations.Count-ne 7){throw 'Expected seven actual particle callback table allocations.'}
[IO.File]::WriteAllText((Join-Path $out 'jpa_callback_allocations.inc'),(($allocations|ForEach-Object Value)-join "`n"))
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
$compileArgs=@('-std=c++17','-O2','-UNDEBUG','-target','x86_64-windows-gnu')
foreach($relative in @('native\src','native\include')){$compileArgs+=('-I'+(Join-Path $p $relative))}
$compileArgs+=('-I'+$out)
$compileArgs+=@((Join-Path $n 'src\jpa_archive_tests.cpp'),'-lbcrypt','-o',$exe)
& (Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ @compileArgs *> (Join-Path $out 'build.log')
if($LASTEXITCODE){Get-Content -LiteralPath (Join-Path $out 'build.log') -Tail 25;exit $LASTEXITCODE}
& $exe 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs'
exit $LASTEXITCODE
