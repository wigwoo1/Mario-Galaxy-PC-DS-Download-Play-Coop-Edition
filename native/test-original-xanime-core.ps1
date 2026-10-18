param([switch]$UseExistingGeneratedSources)
$stationedSource=Get-Content -LiteralPath (Join-Path (Split-Path $PSScriptRoot -Parent) 'Petari\src\Game\System\StationedFileInfo.cpp') -Raw
$stationedArchives=[regex]::Matches($stationedSource,'"/ObjectData/[^"\r\n]+\.arc"') | ForEach-Object {$_.Value} | Select-Object -Unique
if(-not $stationedArchives){throw 'Stationed archive test inventory missing.'}
[IO.File]::WriteAllText((Join-Path $PSScriptRoot 'build\btk-stationed-archives.inc'),(($stationedArchives | ForEach-Object {$_+','}) -join "`n"))
if(!$UseExistingGeneratedSources){
& "$PSScriptRoot\generate-jmath-inline.ps1"
& "$PSScriptRoot\generate-material-struct.ps1"
& "$PSScriptRoot\generate-model-access.ps1"
& "$PSScriptRoot\generate-texture-buffer-size.ps1"
& "$PSScriptRoot\generate-model-orientation.ps1"
& "$PSScriptRoot\generate-vector-projection.ps1"
& "$PSScriptRoot\generate-memory-copy.ps1"
& "$PSScriptRoot\probe-original-gd-effects.ps1"
& "$PSScriptRoot\generate-model-fur-check.ps1"
& "$PSScriptRoot\generate-model-env-init.ps1"
& "$PSScriptRoot\generate-draw-buffer-inline.ps1"
}
$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$exe=Join-Path $n 'build\original-xanime-core-tests.exe'
$log=Join-Path $n 'build\original-xanime-core-tests.log'
$env:ZIG_GLOBAL_CACHE_DIR="$n\build\zig-cache-original-xanime-core"
if(!$UseExistingGeneratedSources){
 & "$n\generate-model-flags.ps1"
 & "$n\generate-shape-table-adapter.ps1"
}
$env:ZIG_GLOBAL_CACHE_DIR="$n\build\zig-cache-original-xanime-core"
$line=Get-Content "$n\test-unit-heap-live.ps1" | Where-Object {$_ -like '& *zig.exe*'}
$providers=' "$p\Petari\src\JSystem\J3DGraphBase\J3DStruct.cpp" "$n\src\generated\J3DPacket.cpp" "$n\src\generated\ModelSyncKernel.cpp" "$n\src\generated\ModelFurCheck.cpp" "$n\src\generated\MemoryCopyKernel.cpp" "$n\src\generated\VectorProjectionKernel.cpp" "$n\src\generated\ModelOrientationKernel.cpp" "$n\src\generated\TextureBufferSizeKernel.cpp" '
foreach($unit in @('J3DCluster','J3DSkinDeform','J3DModel','XanimeCore','J3DJointFactory','J3DShapeFactory','J3DJoint','J3DJointTree','J3DMtxBuffer','J3DMaterial','J3DMaterialFactory','J3DMatBlock','J3DTevs','J3DSys','J3DGD','J3DTransformPort','J3DMaterialAnm','J3DAnimation','J3DShape','J3DShapeDraw','J3DShapeMtx','J3DVertex','J3DDrawBuffer','JMath','JMATrigonometric','JUTNameTab')){
 $providers+=' "'+"$n\build\actor-runtime-probe\$unit.obj"+'" '
}
foreach($file in @('j3d_no_animation_base','j3d_draw_buffer_cleanup','native_bck_animation','vector_blend_port','native_material_factory_port','gd_storage','gpu_resources','sdk_cache_port','sdk_matrix_port','gx_array_port','gx_indirect_port','gx_draw_port','gx_vertex_cache_port','gx_texture_cache_port','gx_test_data','gx_tev_port','j3d_shape_matrix_dispatch','jut_console_port','os_critical_port','j3d_packet_base_port')){
 $providers+=' "'+"$n\src\$file.cpp"+'" '
}
foreach($unit in @('J3DMaterial','J3DMaterialAnm','J3DAnimation')){
 $source="$n\src\generated\$unit.cpp"
 if(!(Test-Path $source)){$source="$p\Petari\src\JSystem\J3DGraphAnimator\$unit.cpp"}
 if($unit -eq "J3DMaterial"){$source="$p\Petari\src\JSystem\J3DGraphBase\J3DMaterial.cpp"}
 $providers=$providers.Replace('"'+ "$n\build\actor-runtime-probe\$unit.obj"+'"','"'+$source+'"')
}
$providers+= ' "$n\src\generated\GDGeometry.cpp" "$n\src\generated\GDTexture.cpp" "$n\build\actor-runtime-probe\GDTransform.obj" -lbcrypt -ld3d11 -ldxgi -ld3dcompiler_47 -luser32 -lgdi32 -ldbghelp '
$providers+= ' "$n\src\generated\TextureObjectLodKernel.cpp" ' 
$providers+= ' "$n\src\generated\AnimationHelperKernel.cpp" "$p\Petari\src\JSystem\J3DGraphAnimator\J3DModelData.cpp" "$p\Petari\src\JSystem\J3DGraphAnimator\J3DMaterialAttach.cpp" "$p\Petari\src\JSystem\J3DGraphAnimator\J3DShapeTable.cpp" '
$archiveProviders=(Get-Content "$n\test-original-archive-mount.ps1" | Where-Object {$_ -like '$providers=*'} | Select-Object -First 1).Substring(11)
$archiveProviders=$archiveProviders.Trim().Trim("'")
foreach($duplicate in @('jut_console_port','sdk_cache_port','gpu_resources')){$archiveProviders=$archiveProviders.Replace('"$n\src\'+$duplicate+'.cpp"','')}
$providers+=' '+$archiveProviders+' '
foreach($source in @('"$n\src\generated\JMapInfo.cpp"','"$p\Petari\src\JSystem\JGadget\hashcode.cpp"','"$n\src\string_equal_case_port.cpp"')){
 $leaf=[IO.Path]::GetFileName($source.Trim('"'))
 if(-not $providers.Contains($leaf)){$providers+=' '+$source+' '}
}
$writerManifest=Join-Path $n 'build\startup-verification-20261014\gd-writer-cached-providers.txt'
if(Test-Path -LiteralPath $writerManifest){
 foreach($oldWriter in (Get-Content -LiteralPath $writerManifest)){
  $newWriter=Join-Path $n ('build\actor-runtime-gd-writer-tls-20261014\'+[IO.Path]::GetFileName($oldWriter))
  if(!(Test-Path -LiteralPath $newWriter)){throw "Missing thread-local GD test provider: $newWriter"}
  $providers=$providers.Replace('"'+$oldWriter+'"','"'+$newWriter+'"')
  $providers=$providers.Replace('"$n'+$oldWriter.Substring($n.Length)+'"','"'+$newWriter+'"')
 }
}
foreach($unit in @('J3DTevs','J3DDrawBuffer','JUTTexture','J3DModelLoader')){
 $replacement=Join-Path $n ('build\actor-runtime-texture-pointers-20261014\'+$unit+'.obj')
 if(Test-Path -LiteralPath $replacement){
  foreach($directory in @('actor-runtime-probe','actor-runtime-gd-writer-tls-20261014')){
   $old=Join-Path $n ('build\'+$directory+'\'+$unit+'.obj')
   $providers=$providers.Replace('"'+$old+'"','"'+$replacement+'"').Replace('"$n\build\'+$directory+'\'+$unit+'.obj"','"'+$replacement+'"')
  }
 }
}
$matrixProviderMatches=[regex]::Matches($providers,'"[^"\r\n]*\\J3DMtxBuffer\.obj"')
if($matrixProviderMatches.Count-ne 1){throw 'Expected one matrix test provider.'}
$providers=$providers.Replace($matrixProviderMatches[0].Value,'"'+(Join-Path $n 'src\generated\J3DMtxBuffer.cpp')+'"')
$line=$line.Replace('src\unit_heap_live_tests.cpp','src\original_xanime_core_tests.cpp').Replace(' -o ',$providers+' -o ').Replace('unit-heap-live-tests','original-xanime-core-tests')
$line=$line.Replace(' c++ ',' c++ -DGALAXY_ORIGINAL_GD_GEOMETRY ')
$line=$line.Replace(" -std=c++17 "," -std=c++17 -DGALAXY_NATIVE_MATERIAL_ANIMATOR_POINTERS ")
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$n\build\original-xanime-core-tests.log" -Tail 30;exit 1}
& "$n\build\original-xanime-core-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs' | Tee-Object "$n\build\material-creation-run.log"
exit $LASTEXITCODE







































