$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$p=Split-Path $n -Parent
$tmp=Join-Path $env:TEMP 'galaxy-dsp-audio-command-hle-tests'
New-Item -ItemType Directory -Force -Path $tmp | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $tmp 'zig-cache'
$log=Join-Path $tmp 'build.log'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "$n\src\dsp_audio_command_hle_tests.cpp" "$n\src\dsp_audio_command_hle.cpp" -o (Join-Path $tmp 'dsp-audio-command-hle-tests.exe') *> $log
if($LASTEXITCODE){Get-Content $log -Tail 40;exit $LASTEXITCODE}
& (Join-Path $tmp 'dsp-audio-command-hle-tests.exe')
exit $LASTEXITCODE
