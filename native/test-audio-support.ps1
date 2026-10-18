$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\audio_support_tests.cpp" "$n\src\native_audio_support_port.cpp" -o "$n\build\audio-support-tests.exe" *> "$n\build\audio-support-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\audio-support-tests.log" -Tail 20;exit 1}
& "$n\build\audio-support-tests.exe"
exit $LASTEXITCODE
