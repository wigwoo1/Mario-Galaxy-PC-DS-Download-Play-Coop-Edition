& "$PSScriptRoot\generate-light-blend.ps1"
& "$PSScriptRoot\generate-actor-light-load.ps1"
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\actor_light_load_tests.cpp" "$n\src\gpu_resources.cpp" "$n\src\gd_storage.cpp" "$n\src\gx_light_object_port.cpp" "$n\src\generated\LightObjectKernel.cpp" "$n\src\generated\ActorLightLoadKernel.cpp" "$n\src\gx_indirect_port.cpp" "$n\src\generated\LightBlendKernel.cpp" "$n\src\generated\LightBlendScalarFixture.cpp" "$n\src\sdk_matrix_port.cpp" -ld3dcompiler_47 -ld3d11 -ldxgi -o "$n\build\actor-light-load-tests.exe" *> "$n\build\actor-light-load-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\actor-light-load-tests.log" -Tail 15;exit 1}
& "$n\build\actor-light-load-tests.exe"
exit $LASTEXITCODE












