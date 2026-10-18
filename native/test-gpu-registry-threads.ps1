$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out=Join-Path $n 'build\gpu-registry-thread-tests';New-Item -ItemType Directory -Force -Path $out | Out-Null
$exe=Join-Path $out 'gpu-registry-thread-tests.exe'
& (Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu ('-I'+(Join-Path $n 'include')) ('-I'+(Join-Path $n 'include-runtime')) ('-I'+(Join-Path $p 'Petari\libs\RVL_SDK\include')) -include revolution/types.h (Join-Path $n 'src\gpu_registry_thread_tests.cpp') (Join-Path $n 'src\gpu_resources.cpp') -o $exe *> (Join-Path $out 'build.log')
if($LASTEXITCODE){Get-Content -LiteralPath (Join-Path $out 'build.log') -Tail 20;exit $LASTEXITCODE}
& $exe
exit $LASTEXITCODE