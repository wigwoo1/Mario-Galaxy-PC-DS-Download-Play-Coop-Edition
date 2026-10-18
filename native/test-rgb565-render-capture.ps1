$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
& "$n\generate-texture-copy-setup.ps1"
& "$n\generate-texture-copy-emit.ps1"
& "$n\probe-screen-capture.ps1"
& "$n\generate-texture-capture.ps1"
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\rgb565_render_capture_tests.cpp" "$n\src\gpu_resources.cpp" "$n\src\gd_storage.cpp" "$n\src\gx_texture_copy_port.cpp" "$n\src\gx_display_copy_port.cpp" "$n\src\generated\DisplayCopySetupKernel.cpp" "$n\src\generated\DisplayCopyEmitKernel.cpp" "$n\src\generated\JUTTextureCaptureKernel.cpp" "$n\src\generated\TextureCopySetupKernel.cpp" "$n\src\generated\TextureCopyEmitKernel.cpp" -ld3d11 -ld3dcompiler_47 -ldxgi -luser32 -lbcrypt -o "$n\build\rgb565-render-capture-tests.exe" *> "$n\build\rgb565-render-capture-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\rgb565-render-capture-tests.log" -Tail 20;exit 1}
& "$n\build\rgb565-render-capture-tests.exe"
exit $LASTEXITCODE






