$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$n\generate-native-speaker-wave.ps1"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\RVL_SDK\include" -target x86_64-windows-gnu "$n\src\speaker_retail_scan.cpp" "$n\src\generated\SpeakerWaveGettersFixture.cpp" -lbcrypt -o "$n\build\speaker-retail-scan.exe" *> "$n\build\speaker-retail-scan-build.log"
if($LASTEXITCODE){Get-Content "$n\build\speaker-retail-scan-build.log" -Tail 10;exit $LASTEXITCODE}
& "$n\build\speaker-retail-scan.exe" "D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs" *> "$n\build\speaker-retail-scan.log"
Get-Content "$n\build\speaker-retail-scan.log"
exit $LASTEXITCODE



