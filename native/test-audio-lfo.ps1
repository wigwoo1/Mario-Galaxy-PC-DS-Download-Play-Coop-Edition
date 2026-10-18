$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "$n\src\audio_lfo_tests.cpp" "$p\Petari\src\JSystem\JAudio2\JASLfo.cpp" "$p\Petari\src\JSystem\JAudio2\JASOscillator.cpp" "$n\build\actor-runtime-probe\JMATrigonometric.obj" -o "$n\build\audio-lfo-tests.exe"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\audio-lfo-tests.exe"
exit $LASTEXITCODE




