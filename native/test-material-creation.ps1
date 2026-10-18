$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$n\generate-model-flags.ps1"
$line=Get-Content "$n\test-unit-heap-live.ps1" | Where-Object {$_ -like '& *zig.exe*'}
$providers=''
foreach($unit in @('J3DMaterial','J3DMaterialFactory','J3DMatBlock','J3DTevs','J3DSys','J3DStruct','J3DGD','J3DTransformPort','J3DPacket','J3DMaterialAnm','J3DAnimation','J3DShape','J3DShapeDraw','J3DShapeMtx','J3DVertex','J3DDrawBuffer','JMath','JMATrigonometric','JUTNameTab')){
 $providers+=' "'+"$n\build\actor-runtime-probe\$unit.obj"+'" '
}
foreach($file in @('native_material_factory_port','gd_storage','gpu_resources','sdk_cache_port','sdk_matrix_port','gx_array_port','gx_indirect_port','gx_draw_port','gx_vertex_cache_port','gx_texture_cache_port','gx_tev_port','j3d_shape_matrix_dispatch','jut_console_port','os_critical_port','j3d_packet_base_port')){
 $providers+=' "'+"$n\src\$file.cpp"+'" '
}
$providers+=' "'+"$n\src\generated\GDVertexDescriptor.cpp"+'" -lbcrypt -ld3d11 -ldxgi -ld3dcompiler_47 -luser32 -lgdi32 '
$providers+=' "'+"$n\src\generated\TextureObjectLodKernel.cpp"+'" "'+"$n\src\generated\ModelFlagsKernel.cpp"+'" '
$line=$line.Replace('src\unit_heap_live_tests.cpp','src\material_creation_tests.cpp').Replace(' -o ',$providers+' -o ').Replace('unit-heap-live-tests','material-creation-tests')
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$n\build\material-creation-tests.log" -Tail 30;exit 1}
& "$n\build\material-creation-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs' | Tee-Object "$n\build\material-creation-run.log"
exit $LASTEXITCODE




