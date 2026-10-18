$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out=Join-Path $n 'build\rfl-archive-test';New-Item -ItemType Directory -Force -Path $out | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
$exe=Join-Path $out 'rfl-archive-tests.exe'
& (Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu ('-I'+(Join-Path $n 'include')) (Join-Path $n 'src\rfl_archive_tests.cpp') -lbcrypt -o $exe *> (Join-Path $out 'build.log')
if($LASTEXITCODE){Get-Content -LiteralPath (Join-Path $out 'build.log') -Tail 15;exit $LASTEXITCODE}
& $exe 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs'
exit $LASTEXITCODE
