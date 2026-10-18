$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\sound_parameter_tests.cpp" "$p\Petari\src\JSystem\JAudio2\JAISoundParams.cpp" "$p\Petari\src\JSystem\JAudio2\JASSoundParams.cpp" -o "$n\build\sound-parameter-tests.exe"
if($LASTEXITCODE){exit $LASTEXITCODE}
& "$n\build\sound-parameter-tests.exe"
exit $LASTEXITCODE

