& "$PSScriptRoot\generate-native-odh-codec.ps1"
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\odh_codec_tests.cpp" "$n\src\generated\odh.cpp" -o "$n\build\odh-codec-tests.exe" *> "$n\build\odh-codec-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\odh-codec-tests.log" -Tail 20;exit 1}
& "$n\build\odh-codec-tests.exe"
exit $LASTEXITCODE



