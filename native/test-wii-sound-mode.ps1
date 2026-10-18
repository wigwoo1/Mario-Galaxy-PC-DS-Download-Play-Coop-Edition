$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" -include revolution/types.h "$n\src\wii_sound_mode_tests.cpp" "$n\src\wii_sound_mode_port.cpp" -o "$n\build\wii-sound-mode-tests.exe" *> "$n\build\wii-sound-mode-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\wii-sound-mode-tests.log" -Tail 30;exit 1}
& "$n\build\wii-sound-mode-tests.exe"
exit $LASTEXITCODE
