$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -include cmath -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "$n\src\retail_audio_arc_tests.cpp"  "$n\src\generated\AudioArcInterpreterKernel.cpp" "$n\src\bank_chunk_port.cpp" -o "$n\build\retail-audio-arc-tests.exe"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\retail-audio-arc-tests.exe" "$n\build\retail-audio.baa"
exit $LASTEXITCODE






