$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out=Join-Path $n 'build\tev-constants';New-Item -ItemType Directory -Force -Path $out | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $p 'tools\zig-cache'
$argsList=@('-std=c++17','-O2','-UNDEBUG','-target','x86_64-windows-gnu',('-I'+$n+'\include'),('-I'+$n+'\src'),('-I'+$p+'\Petari\include'),('-I'+$p+'\Petari\libs\JSystem\include'),('-I'+$p+'\Petari\libs\RVL_SDK\include'),'-include','revolution/types.h',(Join-Path $n 'src\d3d_tev_constants_tests.cpp'),(Join-Path $n 'src\gpu_resources.cpp'),'-ld3dcompiler_47','-ld3d11','-ldxgi','-o',(Join-Path $out 'd3d-tev-constants-tests.exe'))
& (Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ @argsList *> (Join-Path $out 'build.log')
if($LASTEXITCODE){Get-Content -LiteralPath (Join-Path $out 'build.log') -Tail 20;exit $LASTEXITCODE}
& (Join-Path $out 'd3d-tev-constants-tests.exe') | Tee-Object -FilePath (Join-Path $out 'result.txt')
exit $LASTEXITCODE
