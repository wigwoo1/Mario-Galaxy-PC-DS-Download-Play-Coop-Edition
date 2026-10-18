$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& "$n\generate-native-player-status.ps1"
$env:ZIG_GLOBAL_CACHE_DIR="$n\build\zig-cache-player-status"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -Wno-nullability-completeness -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\player_status_codec_tests.cpp" "$n\src\generated\GameDataPlayerStatus.cpp" "$n\src\generated\StringKernel.cpp" -o "$n\build\player-status-codec-tests.exe" *> "$n\build\player-status-codec-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\player-status-codec-tests.log" -Tail 15;exit 1}
& "$n\build\player-status-codec-tests.exe"
exit $LASTEXITCODE


