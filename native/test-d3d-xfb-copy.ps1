$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out=Join-Path $env:TEMP 'smg-native-d3d-xfb-copy'
New-Item -ItemType Directory -Force -Path $out | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
$exe=Join-Path $out 'd3d-xfb-copy-tests.exe'
$log=Join-Path $out 'd3d-xfb-copy-tests.log'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\d3d_xfb_copy_tests.cpp" "$n\src\gx_submission_port.cpp" "$n\src\gx_test_data.cpp" "$n\src\gd_storage.cpp" "$n\src\gx_depth_peek_port.cpp" "$n\src\gx_efb_readback_port.cpp" "$n\src\gx_draw_sync_completion_port.cpp" "$n\src\gx_draw_completion_port.cpp" "$n\src\gpu_resources.cpp" "$n\src\generated\DisplayCopySetupKernel.cpp" "$n\src\generated\DisplayCopyEmitKernel.cpp" "$n\src\generated\TextureCopySetupKernel.cpp" "$n\src\gx_display_copy_port.cpp" -ld3dcompiler_47 -ld3d11 -ldxgi -o $exe *> $log
if($LASTEXITCODE){Get-Content -LiteralPath $log -Tail 30;exit 1}
& $exe
exit $LASTEXITCODE
