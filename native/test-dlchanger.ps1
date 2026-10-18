$ErrorActionPreference='Stop'
$projectRoot=Split-Path $PSScriptRoot -Parent
$petariRoot=Join-Path $projectRoot 'Petari'
$compiler=Join-Path $projectRoot 'tools\zig-x86_64-windows-0.16.0\zig.exe'
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $projectRoot 'tools\zig-cache'
$build=Join-Path $PSScriptRoot 'build'
New-Item -ItemType Directory -Force -Path $build | Out-Null
& $compiler c++ -std=c++17 -target x86_64-windows-gnu "-I$PSScriptRoot\include-runtime" "-I$PSScriptRoot\include" "-I$petariRoot\include" "-I$petariRoot\libs\JSystem\include" "-I$petariRoot\libs\RVL_SDK\include" -include revolution/types.h (Join-Path $PSScriptRoot 'src\dlchanger_tests.cpp') -o (Join-Path $build 'dlchanger-tests.exe') *> (Join-Path $build 'dlchanger-tests.log')
if($LASTEXITCODE -ne 0){Get-Content -LiteralPath (Join-Path $build 'dlchanger-tests.log') -Tail 20;exit 1}
& (Join-Path $build 'dlchanger-tests.exe') *> (Join-Path $build 'dlchanger-tests-run.log')
$testStatus=$LASTEXITCODE
Get-Content -LiteralPath (Join-Path $build 'dlchanger-tests-run.log')
exit $testStatus
