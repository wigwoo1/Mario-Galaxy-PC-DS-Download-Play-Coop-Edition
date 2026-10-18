& "$PSScriptRoot\generate-texture-buffer-size.ps1"
& "$PSScriptRoot\generate-model-orientation.ps1"
& "$PSScriptRoot\generate-vector-projection.ps1"
& "$PSScriptRoot\generate-memory-copy.ps1"
& "$PSScriptRoot\probe-shape-packet-userdata.ps1"
& "$PSScriptRoot\probe-original-model-x.ps1"
& "$PSScriptRoot\probe-original-gd-effects.ps1"
& "$PSScriptRoot\generate-model-fur-check.ps1"
& "$PSScriptRoot\generate-model-env-init.ps1"
& "$PSScriptRoot\generate-draw-buffer-inline.ps1"
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$n\generate-model-flags.ps1"
& "$n\generate-shape-table-adapter.ps1"
$line=Get-Content "$n\test-unit-heap-live.ps1" | Where-Object {$_ -like '& *zig.exe*'}
$providers=' "$n\build\actor-runtime-probe\ShapePacketUserData.obj" "$n\src\generated\ModelEffectChecks.cpp"  "$n\src\generated\ModelFurCheck.cpp" "$n\src\generated\MemoryCopyKernel.cpp" "$n\src\generated\VectorProjectionKernel.cpp" "$n\src\generated\ModelOrientationKernel.cpp" "$n\src\generated\TextureBufferSizeKernel.cpp" "$n\build\actor-runtime-probe\J3DModelX.obj" "$n\build\actor-runtime-probe\GDPixel.obj" "$n\build\actor-runtime-probe\GDTev.obj" "$n\build\actor-runtime-probe\GDIndirect.obj" "$n\build\actor-runtime-probe\GDLight.obj" "$p\Petari\src\Game\Util\SchedulerUtil.cpp" '
foreach($unit in @('J3DCluster','J3DSkinDeform','J3DModel','XanimeCore','J3DJointFactory','J3DShapeFactory','J3DJoint','J3DJointTree','J3DMtxBuffer','J3DMaterial','J3DMaterialFactory','J3DMatBlock','J3DTevs','J3DSys','J3DStruct','J3DGD','J3DTransformPort','J3DPacket','J3DMaterialAnm','J3DAnimation','J3DShape','J3DShapeDraw','J3DShapeMtx','J3DVertex','J3DDrawBuffer','JMath','JMATrigonometric','JUTNameTab')){
 $providers+=' "'+"$n\build\actor-runtime-probe\$unit.obj"+'" '
}
foreach($file in @('j3d_no_animation_base','j3d_draw_buffer_cleanup','native_bck_animation','vector_blend_port','native_material_factory_port','gd_storage','gpu_resources','sdk_cache_port','sdk_matrix_port','gx_array_port','gx_indirect_port','gx_draw_port','gx_vertex_cache_port','gx_texture_cache_port','gx_tev_port','j3d_shape_matrix_dispatch','jut_console_port','os_critical_port','j3d_packet_base_port')){
 $providers+=' "'+"$n\src\$file.cpp"+'" '
}
$providers+= ' "$n\src\generated\GDGeometry.cpp" "$n\src\generated\GDTexture.cpp" "$n\build\actor-runtime-probe\GDTransform.obj" -lbcrypt -ld3d11 -ldxgi -ld3dcompiler_47 -luser32 -lgdi32 '
$providers+= ' "$n\src\generated\TextureObjectLodKernel.cpp" ' 
$providers+= ' "$n\src\generated\AnimationHelperKernel.cpp" "$p\Petari\src\JSystem\J3DGraphAnimator\J3DModelData.cpp" "$p\Petari\src\JSystem\J3DGraphAnimator\J3DMaterialAttach.cpp" "$p\Petari\src\JSystem\J3DGraphAnimator\J3DShapeTable.cpp" '
$archiveProviders=(Get-Content "$n\test-original-archive-mount.ps1" | Where-Object {$_ -like '$providers=*'} | Select-Object -First 1).Substring(11)
$archiveProviders=$archiveProviders.Trim().Trim("'")
foreach($duplicate in @('jut_console_port','sdk_cache_port','gpu_resources')){$archiveProviders=$archiveProviders.Replace('"$n\src\'+$duplicate+'.cpp"','')}
$providers+=' '+$archiveProviders+' '
$line=$line.Replace('src\unit_heap_live_tests.cpp','src\original_xanime_core_tests.cpp').Replace(' -o ',$providers+' -o ').Replace('unit-heap-live-tests','original-model-x-link-check')
$line=$line.Replace(' c++ ',' c++ -DGALAXY_ORIGINAL_GD_GEOMETRY ')
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$n\build\original-model-x-link-check.log" -Tail 30;exit 1}
& "$n\build\original-model-x-link-check.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs' | Tee-Object "$n\build\material-creation-run.log"
exit $LASTEXITCODE






























