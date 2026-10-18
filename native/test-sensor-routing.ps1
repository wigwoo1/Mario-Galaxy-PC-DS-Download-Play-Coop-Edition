$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$n\generate-sensor-routing-fixture.ps1"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h -include revolution/os.h "$n\src\sensor_routing_tests.cpp" "$n\src\generated\SensorRoutingFixture.cpp" "$n\src\generated\CameraNameObjKernel.cpp" "$p\Petari\src\Game\NameObj\NameObjRegister.cpp" "$p\Petari\src\Game\LiveActor\HitSensor.cpp" "$p\Petari\src\Game\LiveActor\SensorHitChecker.cpp" -o "$n\build\sensor-routing-tests.exe" *> "$n\build\sensor-routing-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\sensor-routing-tests.log" -Tail 20;exit 1}
& "$n\build\sensor-routing-tests.exe"
exit $LASTEXITCODE
