$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
& "$n\generate-model-flags.ps1"
$exe=Join-Path $n 'build\material-creation-tests.exe'
$log=Join-Path $n 'build\material-creation-tests.log'
$line=Get-Content "$n\test-unit-heap-live.ps1" | Where-Object {$_ -like '& *zig.exe*'}
$providers=''
foreach($unit in @('J3DMaterial','J3DMaterialFactory','J3DMatBlock','J3DTevs','J3DSys','J3DStruct','J3DGD','J3DTransformPort','J3DPacket','J3DMaterialAnm','J3DAnimation','J3DShape','J3DShapeDraw','J3DShapeMtx','J3DVertex','J3DDrawBuffer','JMath','JMATrigonometric','JUTNameTab')){
 $providers+=' "'+"$n\build\actor-runtime-probe\$unit.obj"+'" '
}
foreach($file in @('native_material_factory_port','gd_storage','gpu_resources','sdk_cache_port','sdk_matrix_port','gx_array_port','gx_indirect_port','gx_draw_port','gx_vertex_cache_port','gx_texture_cache_port','gx_test_data','gx_tev_port','j3d_shape_matrix_dispatch','jut_console_port','os_critical_port','j3d_packet_base_port')){
 $providers+=' "'+"$n\src\$file.cpp"+'" '
}
foreach($unit in @('J3DMaterialAnm','J3DAnimation')){
 $source=Join-Path $n ('src\generated\'+$unit+'.cpp')
 if(!(Test-Path -LiteralPath $source)){$source=Join-Path $p ('Petari\src\JSystem\J3DGraphAnimator\'+$unit+'.cpp')}
 $providers=$providers.Replace(('"'+"$n\build\actor-runtime-probe\$unit.obj"+'"'),('"'+$source+'"'))
}
$providers+=' "'+"$p\Petari\src\JSystem\J3DGraphAnimator\J3DMaterialAttach.cpp"+'" '
$providers+=' "'+"$n\src\generated\GDVertexDescriptor.cpp"+'" -lbcrypt -ld3d11 -ldxgi -ld3dcompiler_47 -luser32 -lgdi32 '
$providers+=' "'+"$n\src\generated\TextureObjectLodKernel.cpp"+'" "'+"$n\src\generated\ModelFlagsKernel.cpp"+'" '
$writerManifest=Join-Path $n 'build\startup-verification-20261014\gd-writer-cached-providers.txt'
if(Test-Path -LiteralPath $writerManifest){
 foreach($old in Get-Content -LiteralPath $writerManifest){
  $replacement=Join-Path $n ('build\actor-runtime-gd-writer-tls-20261014\'+[IO.Path]::GetFileName($old))
  if(!(Test-Path -LiteralPath $replacement)){throw "Missing current GD writer provider: $replacement"}
  $providers=$providers.Replace(('"'+$old+'"'),('"'+$replacement+'"'))
 }
}
foreach($unit in @('J3DTevs','J3DDrawBuffer')){
 $replacement=Join-Path $n ('build\actor-runtime-texture-pointers-20261014\'+$unit+'.obj')
 if(Test-Path -LiteralPath $replacement){
  foreach($directory in @('actor-runtime-probe','actor-runtime-gd-writer-tls-20261014')){
   $old=Join-Path $n ('build\'+$directory+'\'+$unit+'.obj')
   $providers=$providers.Replace(('"'+$old+'"'),('"'+$replacement+'"'))
  }
 }
}
$line=$line.Replace('src\unit_heap_live_tests.cpp','src\material_creation_tests.cpp').Replace(' -o ',$providers+' -o ').Replace('unit-heap-live-tests','material-creation-tests')
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$n\build\material-creation-tests.log" -Tail 30;exit 1}
& "$n\build\material-creation-tests.exe" 'D:\Desktop\Super Mario Galaxy (USA, Canada) (En,Fr,Es).wbfs' | Tee-Object "$n\build\material-creation-run.log"
exit $LASTEXITCODE




