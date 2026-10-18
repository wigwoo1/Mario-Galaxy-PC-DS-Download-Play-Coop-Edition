$ErrorActionPreference='Stop'
& "$PSScriptRoot\generate-light-blend.ps1"
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\light_blend_tests.cpp" "$n\src\generated\LightBlendKernel.cpp" "$n\src\generated\PointLightBlendFixtureKernel.cpp" "$n\src\generated\LightBlendScalarFixture.cpp" -o "$n\build\light-blend-tests.exe" *> "$n\build\light-blend-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\light-blend-tests.log" -Tail 15;exit 1}
& "$n\build\light-blend-tests.exe"
exit $LASTEXITCODE


