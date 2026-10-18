$ErrorActionPreference='Stop'
$native=$PSScriptRoot;$project=Split-Path $native -Parent
$output=Join-Path $env:TEMP 'galaxy-scene-executor-cleanup-test'
New-Item -ItemType Directory -Force -Path $output|Out-Null
$source=Get-Content -LiteralPath "$project\Petari\src\Game\NameObj\NameObjListExecutor.cpp" -Raw
$end=$source.IndexOf('template MR::Vector');if($end-lt 0){throw 'Executor constructor/destructor extraction boundary missing.'}
$kernel=Join-Path $output 'actual-cleanup.cpp'
[IO.File]::WriteAllText($kernel,$source.Substring(0,$end))
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $env:TEMP 'galaxy-prepresent-test-cache'
$exe=Join-Path $output 'cleanup-tests.exe';$log=Join-Path $output 'build.log'
& "$project\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$native\include-runtime" "-I$native\include" "-I$project\Petari\include" "-I$project\Petari\libs\RVL_SDK\include" "-I$project\Petari\libs\JSystem\include" "-I$project\Petari\libs\nw4r\include" -include revolution/types.h "$native\src\scene_executor_cleanup_tests.cpp" $kernel -o $exe *> $log
if($LASTEXITCODE){Get-Content $log -Tail 20;exit 1}
& $exe
exit $LASTEXITCODE
