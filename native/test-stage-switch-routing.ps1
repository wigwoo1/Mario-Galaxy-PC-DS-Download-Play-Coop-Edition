& "$PSScriptRoot\generate-jmap-runtime.ps1"
$ErrorActionPreference='Stop'
& "$PSScriptRoot\generate-stage-switch-lifetime.ps1"
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -ffunction-sections -fdata-sections "-Wl,--gc-sections" -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\stage_switch_routing_tests.cpp" "$n\src\generated\StageSwitchRoutingFixture.cpp" "$n\src\generated\CameraNameObjKernel.cpp" "$p\Petari\src\Game\NameObj\NameObjRegister.cpp" "$p\Petari\src\Game\Map\SleepController.cpp" "$n\src\generated\JMapInfo.cpp" "$n\src\string_equal_case_port.cpp" "$p\Petari\src\JSystem\JGadget\hashcode.cpp" -o "$n\build\stage-switch-routing-tests.exe" *> "$n\build\stage-switch-routing-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\stage-switch-routing-tests.log" -Tail 20;exit 1}
& "$n\build\stage-switch-routing-tests.exe"
exit $LASTEXITCODE








