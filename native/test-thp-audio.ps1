$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$build=Join-Path $n 'build';$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $build 'zig-cache'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\thp_audio_tests.cpp" "$n\src\thp_audio_port.cpp" -o "$build\thp-audio-tests.exe" *> "$build\thp-audio-tests.log"
if($LASTEXITCODE){Get-Content "$build\thp-audio-tests.log" -Tail 25;exit $LASTEXITCODE}
& "$build\thp-audio-tests.exe"
exit $LASTEXITCODE
