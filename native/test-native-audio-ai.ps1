$ErrorActionPreference='Stop'
$n=$PSScriptRoot
$p=Split-Path $n -Parent
$env:TEMP=Join-Path $n 'build\tmp'
$env:TMP=$env:TEMP
New-Item -ItemType Directory -Force $env:TEMP | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $n 'build\native-audio-ai-cache'
$exe=Join-Path $n 'build\native-audio-ai-tests.exe'
$log=Join-Path $n 'build\native-audio-ai-tests.log'
$previousCompilerErrorActionPreference=$ErrorActionPreference
$ErrorActionPreference='Continue'
& (Join-Path $p 'tools\zig-x86_64-windows-0.16.0\zig.exe') c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu `
  "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\RVL_SDK\include" `
  (Join-Path $n 'src\native_audio_ai_tests.cpp') `
  (Join-Path $n 'src\native_audio_ai.cpp') `
  (Join-Path $n 'src\dsp_address_space.cpp') `
  -lwinmm -o $exe *> $log
$ErrorActionPreference=$previousCompilerErrorActionPreference
if($LASTEXITCODE -ne 0){Get-Content $log -Tail 24;exit $LASTEXITCODE}
Push-Location $n
try { & $exe; exit $LASTEXITCODE } finally { Pop-Location }
