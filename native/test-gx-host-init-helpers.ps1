$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -ffunction-sections -fdata-sections "-Wl,--gc-sections" -target x86_64-windows-gnu `
 "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" "-I$p\Petari\libs\nw4r\include" `
 -include revolution/types.h -include "$n\include-runtime\revolution\os.h" -include "$n\include-runtime\revolution\gx\GXVert.h" `
 "$n\src\gx_host_init_helpers_tests.cpp" "$n\src\gx_host_service_port.cpp" "$n\src\gx_submission_port.cpp" "$n\src\gx_draw_port.cpp" "$n\src\gx_texture_cache_port.cpp" `
 "$n\src\generated\TextureObjectLodKernel.cpp" "$n\src\gd_storage.cpp" "$n\src\gpu_resources.cpp" `
 -o "$n\build\gx-host-init-helpers-tests.exe" *> "$n\build\gx-host-init-helpers-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\gx-host-init-helpers-tests.log" -Tail 40;exit $LASTEXITCODE}
& "$n\build\gx-host-init-helpers-tests.exe"
exit $LASTEXITCODE
