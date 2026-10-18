$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out=Join-Path $n 'build\texture-pointer-test';New-Item -ItemType Directory -Force -Path $out|Out-Null
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
$exe=Join-Path $out 'texture-pointer-tests.exe'
$compileArgs=@('-std=c++17','-O2','-UNDEBUG','-target','x86_64-windows-gnu')
foreach($rel in @('native\include-runtime','native\include','Petari\include','Petari\libs\JSystem\include','Petari\libs\RVL_SDK\include')){$compileArgs+=('-I'+(Join-Path $p $rel))}
$compileArgs+=@('-include','revolution/types.h',(Join-Path $n 'src\texture_pointer_tests.cpp'),(Join-Path $n 'src\gpu_resources.cpp'),'-o',$exe)
& (Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ @compileArgs *> (Join-Path $out 'build.log')
if($LASTEXITCODE){Get-Content -LiteralPath (Join-Path $out 'build.log') -Tail 18;exit $LASTEXITCODE}
& $exe
exit $LASTEXITCODE
