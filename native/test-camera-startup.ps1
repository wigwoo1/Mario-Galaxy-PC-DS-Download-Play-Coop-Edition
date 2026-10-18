$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;& "$n\probe-screen-capture.ps1"
& "$n\generate-texture-buffer-size.ps1"
& "$n\generate-texture-object.ps1"
& "$n\generate-texture-copy-setup.ps1"
& "$n\generate-texture-copy-emit.ps1"
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$p\tools\zig-x86_64-windows-0.16.0\zig.exe" c++ -std=c++17 -O2 -UNDEBUG -ffunction-sections -fdata-sections "-Wl,--gc-sections" -target x86_64-windows-gnu "-I$n\include-runtime" "-I$n\include" "-I$p\Petari\include" "-I$p\Petari\libs\JSystem\include" "-I$p\Petari\libs\RVL_SDK\include" -include revolution/types.h "$n\src\camera_startup_tests.cpp" "$n\src\generated\AllocatorDispatchKernel.cpp" "$n\src\generated\LayoutDefaultAllocatorKernel.cpp" "$n\src\generated\JKRUnitHeap.cpp" "$n\src\generated\JKRSolidHeap.cpp" "$n\src\generated\JKRExpHeap.cpp" "$n\src\generated\JKRHeapDispatch.cpp" "$n\src\generated\JKRDisposer.cpp" "$p\Petari\src\JSystem\JSupport\JSUList.cpp" "$n\src\os_mutex_port.cpp" "$n\src\native_thread_identity_port.cpp" "$n\src\os_thread_port.cpp" "$n\src\os_message_port.cpp" "$n\src\generated\AsyncJobKernel.cpp" "$n\src\generated\CurrentHeapKernel.cpp" "$n\src\generated\HeapMemoryWatcher.cpp" "$n\src\input_workspace_port.cpp" "$n\src\os_panic_port.cpp" "$n\src\heap_error_port.cpp" "$n\src\heap_arena_port.cpp" "$n\src\generated\JUTTexture.cpp" "$p\Petari\src\JSystem\JUtility\JUTPalette.cpp" "$n\src\generated\TextureBufferSizeKernel.cpp" "$n\src\generated\TextureObjectLodKernel.cpp" "$n\src\gpu_resources.cpp" "$n\src\gd_storage.cpp" "$n\src\sdk_cache_port.cpp" "$n\src\gx_texture_copy_port.cpp" "$n\src\generated\TextureCopySetupKernel.cpp" "$n\src\generated\TextureCopyEmitKernel.cpp" "$n\src\generated\JUTVideo.cpp" "$p\Petari\src\JSystem\JUtility\JUTXfb.cpp" "$n\src\generated\JUTDirectPrint.cpp" "$n\src\vi_port.cpp" "$n\src\os_time_port.cpp" "$n\src\gx_draw_completion_port.cpp" "$n\src\gx_submission_port.cpp" "$n\src\gx_display_copy_port.cpp" "$n\src\generated\DisplayCopySetupKernel.cpp" "$n\src\generated\DisplayCopyEmitKernel.cpp" "$p\Petari\src\Game\Camera\CameraContext.cpp" "$n\src\generated\CameraNameObjKernel.cpp" "$p\Petari\src\Game\NameObj\NameObjRegister.cpp" "$n\src\generated\CameraDisplayConfigKernel.cpp" "$n\src\display_config_port.cpp" "$n\src\sdk_matrix_port.cpp" -ld3d11 -ld3dcompiler_47 -ldxgi -luser32 -lbcrypt -o "$n\build\camera-startup-tests.exe" *> "$n\build\camera-startup-tests.log"
if($LASTEXITCODE){Get-Content "$n\build\camera-startup-tests.log" -Tail 15;exit 1}
& "$n\build\camera-startup-tests.exe"
exit $LASTEXITCODE





















