$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -include math_types.hpp -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "$n\src\audio_dsp_deferred_tests.cpp" "$n\src\generated\AudioDSPDeferredKernel.cpp" -o "$n\build\audio-dsp-deferred-tests.exe"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\audio-dsp-deferred-tests.exe"
exit $LASTEXITCODE






