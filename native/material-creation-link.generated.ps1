$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$n\generate-texture-object.ps1"
& "$n\generate-model-access.ps1"
$line=Get-Content "$n\test-original-kcl-server.ps1" | Where-Object {$_ -like '& *zig.exe*'}
$providers=' "$n\src\native_collision_scene.cpp" "$n\src\generated\CollisionSceneKernel.cpp" "$p\Petari\src\Game\Map\CollisionDirector.cpp" "$p\Petari\src\Game\NameObj\NameObj.cpp" "$p\Petari\src\Game\NameObj\NameObjRegister.cpp" "$n\src\generated\NameObjHolder.cpp" "$n\src\generated\CollisionCodeMetadata.cpp" "$n\src\sensor_hash_port.cpp" "$n\src\generated\StringKernel.cpp" "$n\src\original_live_actor_link_entry.cpp" "$n\src\gpu_resources.cpp" '
$providers+=' "$n\src\gd_storage.cpp" "$n\src\generated\NarrowTextWriter.cpp" "$n\src\generated\TextureObjectLodKernel.cpp" '
$providers+=' "$n\src\sdk_cache_port.cpp" "$n\src\native_arc_port.cpp" "$n\src\native_jkr_archive_port.cpp" "$n\src\generated\HeapCopyKernel.cpp" "$n\src\generated\AramCompletionKernel.cpp" "$n\src\native_jkr_aram_port.cpp" "$n\src\native_dvd_port.cpp" '
$providers+=' "$n\src\aram_read_port.cpp" "$n\src\generated\JKRCompArchive.cpp" "$n\src\generated\JKRAramArchive.cpp" "$n\src\sdk_report_port.cpp" "$n\src\aram_manager_kernel.cpp" "$n\src\generated\AramQueueKernel.cpp" "$n\src\generated\AramCommandKernel.cpp" "$n\src\generated\JKRAramHeap.cpp" "$n\src\generated\JKRAramBlock.cpp" "$n\src\native_material_factory_port.cpp" "$n\src\generated\ModelAccessKernel.cpp" "$n\src\gx_array_port.cpp" '
$providers+=' "$n\src\generated\GDVertexDescriptor.cpp" '
$providers+=' "$n\src\j3d_shape_matrix_dispatch.cpp" '
$providers+=' "$n\src\gx_indirect_port.cpp" "$n\src\gx_draw_port.cpp" "$n\src\gx_vertex_cache_port.cpp" "$n\src\gx_texture_cache_port.cpp" '
$providers+=' "$n\src\gx_tev_port.cpp" '
$providers+=' "$n\src\os_message_port.cpp" "$n\src\os_mutex_port.cpp" "$n\src\native_thread_identity_port.cpp" "$n\src\os_thread_port.cpp" '
$providers+=' "$n\src\os_panic_port.cpp" "$n\src\heap_error_port.cpp" "$n\src\os_zero_memory_port.cpp" "$n\src\heap_arena_port.cpp" "$n\src\input_workspace_port.cpp" "$n\src\resource_font_port.cpp" "$n\src\game_font_backend.cpp" "$n\src\game_message_backend.cpp" "$n\src\generated\AllocatorDispatchKernel.cpp" "$n\src\jut_console_port.cpp" '
$results=Get-Content "$n\build\actor-runtime-probe\results.json" -Raw|ConvertFrom-Json
foreach($r in $results){if($r.ExitCode){throw "Actor source did not compile: $($r.Unit)"};$providers+=' "'+"$n\build\actor-runtime-probe\$($r.Unit).obj"+'" '}
$line=$line.Replace('c++ -std','c++ -DGALAXY_ORIGINAL_MOVEMENT_CONTROLLER -ffunction-sections -fdata-sections "-Wl,--gc-sections" -std').Replace('src\original_kcl_server_tests.cpp','src\material_creation_tests.cpp').Replace(' -o ',$providers+' -lbcrypt -o ').Replace('original-kcl-server-tests.exe','material-creation-tests.exe').Replace('original-kcl-server-tests.log','material-creation-tests.log')
$line=$line.Replace('-include revolution/types.h -include "$n\include-runtime\nw4r\ut\TagProcessorBase.h"','-include revolution/types.h -include "$n\include-runtime\nw4r\ut\TagProcessorBase.h" -include "$n\include-runtime\nw4r\ut\Color.h"')
Invoke-Expression $line
$code=$LASTEXITCODE
[pscustomobject]@{ExitCode=$code;Scope=('Original LiveActor integration with ' + $results.Count + ' actor/utility providers and current collision runtime; link probe only, never executed');Log="$n\build\material-creation-tests.log"}|ConvertTo-Json|Set-Content "$n\build\material-creation-link-status.json"
Get-Content "$n\build\material-creation-tests.log" -Tail 18
if($code){exit $code}; & "$n\build\material-creation-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs'; exit $LASTEXITCODE



































