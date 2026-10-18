$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$n\generate-sensor-group-fixture.ps1"
& "$n\generate-player-distance.ps1"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h -include revolution/os.h "$n\src\hit_sensor_tests.cpp" "$n\src\generated\SensorGroupFixture.cpp" "$n\src\generated\PlayerDistanceKernel.cpp" "$p\Petari\src\Game\LiveActor\HitSensor.cpp" "$p\Petari\src\Game\Util\CollisionPartsFilter.cpp" -o "$n\build\hit-sensor-tests.exe" *> "$n\build\hit-sensor-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\hit-sensor-tests.log" -Tail 20;exit 1}
& "$n\build\hit-sensor-tests.exe"
exit $LASTEXITCODE




