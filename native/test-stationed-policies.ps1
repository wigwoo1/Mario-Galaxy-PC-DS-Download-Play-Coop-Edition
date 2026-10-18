$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$n\generate-native-stationed-loader.ps1"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\stationed_policy_tests.cpp" "$n\src\generated\StationedConditionsFixture.cpp" "$p\Petari\src\Game\System\StationedFileInfo.cpp" -o "$n\build\stationed-policy-tests.exe" *> "$n\build\stationed-policy-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\stationed-policy-tests.log" -Tail 25;exit 1}
& "$n\build\stationed-policy-tests.exe"
exit $LASTEXITCODE



