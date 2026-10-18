$ErrorActionPreference='Stop'
& "$PSScriptRoot\generate-area-light-data.ps1"
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h -include revolution/gx/GXLighting.h "$n\src\area_light_data_tests.cpp" "$n\src\generated\JMapInfo.cpp" "$n\src\generated\AreaLightDataKernel.cpp" "$n\src\generated\LightDataHolderKernel.cpp" "$n\src\generated\CsvLightFixtureKernel.cpp" "$p\Petari\src\JSystem\JGadget\hashcode.cpp" "$n\src\string_equal_case_port.cpp" -lbcrypt -o "$n\build\area-light-data-tests.exe" *> "$n\build\area-light-data-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\area-light-data-tests.log" -Tail 20;exit 1}
& "$n\build\area-light-data-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs'
exit $LASTEXITCODE



