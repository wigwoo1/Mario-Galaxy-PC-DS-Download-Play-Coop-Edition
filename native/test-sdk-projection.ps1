& "$PSScriptRoot\generate-sdk-projection.ps1"
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -target x86_64-windows-gnu "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\sdk_projection_tests.cpp" "$n\src\gpu_resources.cpp" "$n\src\gd_storage.cpp" "$n\src\gx_projection_port.cpp" "$n\src\generated\SdkProjectionKernel.cpp" -ld3dcompiler_47 -ld3d11 -ldxgi -o "$n\build\sdk-projection-tests.exe" *> "$n\build\sdk-projection-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\sdk-projection-tests.log" -Tail 15;exit 1}
& "$n\build\sdk-projection-tests.exe"
exit $LASTEXITCODE











