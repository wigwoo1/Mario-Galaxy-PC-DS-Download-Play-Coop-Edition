$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -include cmath -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "$n\src\game_instrument_param_tests.cpp"  "$n\src\generated\GameInstrumentParamKernel.cpp" "$p\Petari\src\JSystem\JAudio2\JASSoundParams.cpp" -o "$n\build\game-instrument-param-tests.exe"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\game-instrument-param-tests.exe"
exit $LASTEXITCODE





