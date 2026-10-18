$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$out=Join-Path $env:TEMP 'smg-native-d3d-game-host'
New-Item -ItemType Directory -Force -Path $out | Out-Null
$env:ZIG_GLOBAL_CACHE_DIR=Join-Path $env:TEMP 'smg-zig-cache-game-host-test'
$exe=Join-Path $out 'd3d-game-host-tests.exe'
$log=Join-Path $out 'd3d-game-host-tests.log'
$savedErrorActionPreference=$ErrorActionPreference;$ErrorActionPreference='Continue'
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$n\src" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\d3d_game_host_tests.cpp" "$n\src\gx_submission_port.cpp" "$n\src\gx_test_data.cpp" "$n\src\gd_storage.cpp" "$n\src\gx_depth_peek_port.cpp" "$n\src\gx_efb_readback_port.cpp" "$n\src\gx_draw_sync_completion_port.cpp" "$n\src\gx_draw_completion_port.cpp" "$n\src\gpu_resources.cpp" "$n\src\generated\DisplayCopySetupKernel.cpp" "$n\src\generated\DisplayCopyEmitKernel.cpp" "$n\src\generated\TextureCopySetupKernel.cpp" "$n\src\gx_display_copy_port.cpp" "$n\src\gx_texture_copy_port.cpp" "$n\src\generated\TextureCopyEmitKernel.cpp" "$n\src\native_keyboard_kpad.cpp" "$n\src\native_kpad_input.cpp" "$n\src\native_wpad_port.cpp" "$n\src\vi_port.cpp" -ld3dcompiler_47 -ld3d11 -ldxgi -luser32 -o $exe *> $log
$compileExitCode=$LASTEXITCODE;$ErrorActionPreference=$savedErrorActionPreference
if($compileExitCode){Get-Content -LiteralPath $log -Tail 40;exit 1}
& $exe
exit $LASTEXITCODE
