$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& "$n\generate-native-config-mii.ps1"
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\RVLFaceLib\include" -include revolution/types.h "$n\src\config_mii_codec_tests.cpp" "$n\src\generated\ConfigDataMii.cpp" "$n\src\generated\StringKernel.cpp" -o "$n\build\config-mii-codec-tests.exe" *> "$n\build\config-mii-codec-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\config-mii-codec-tests.log" -Tail 15;exit 1}
& "$n\build\config-mii-codec-tests.exe"
exit $LASTEXITCODE





