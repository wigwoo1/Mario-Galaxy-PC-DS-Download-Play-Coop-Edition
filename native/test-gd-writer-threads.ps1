$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out=Join-Path $n 'build\gd-writer-thread-test';New-Item -ItemType Directory -Force -Path $out|Out-Null
$exe=Join-Path $out 'gd-writer-thread-tests.exe'
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
$compileArgs=@('-std=c++17','-O2','-UNDEBUG','-target','x86_64-windows-gnu')
foreach($relative in @('native\include-runtime','native\include','Petari\libs\RVL_SDK\include')){$compileArgs+=('-I'+(Join-Path $p $relative))}
$compileArgs+=@('-include','revolution/types.h')
foreach($source in @('gd_writer_thread_tests','gx_submission_port','gd_storage','gpu_resources')){$compileArgs+=(Join-Path $n ('src\'+$source+'.cpp'))}
$compileArgs+=@('-o',$exe)
& (Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ @compileArgs *> (Join-Path $out 'build.log')
if($LASTEXITCODE){Get-Content -LiteralPath (Join-Path $out 'build.log') -Tail 20;exit $LASTEXITCODE}
& $exe
exit $LASTEXITCODE
