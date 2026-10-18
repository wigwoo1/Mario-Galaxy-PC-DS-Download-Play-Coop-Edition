$ErrorActionPreference='Continue'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$out="$n\build\actor-runtime-probe";$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
$light=Get-Content -Raw "$p\Petari\libs\RVL_SDK\include\revolution\gd\GDLight.h"
if(!$light.Contains('__cntlzw(id)')){throw 'Original light index intrinsic changed'}
$light=$light.Replace('__cntlzw(id)','(u32(id)==0 ? 32u : u32(__builtin_clz(u32(id))))')
New-Item -ItemType Directory -Force "$n\include-runtime\revolution\gd" | Out-Null
Set-Content "$n\include-runtime\revolution\gd\GDLight.h" $light
$line=(Get-Content "$n\probe-actor-runtime.ps1" | Where-Object {$_ -like '& *zig.exe*'}).Replace(' c++ ',' c++ -x c++ ')
foreach($unit in @('GDPixel','GDTev','GDIndirect','GDLight','GDGeometry','GDTexture','GDTransform')){
 $source="$p\Petari\src\RVL_SDK\gd\$unit.c"
 if($unit -eq 'GDGeometry'){
  $geometry=Get-Content -Raw $source
  if(!$geometry.Contains('OSCachedToPhysical(base_ptr)')){throw 'Original geometry array address changed'}
  $geometry=$geometry.Replace('OSCachedToPhysical(base_ptr)','Native::gpuResources().byteAddress(base_ptr)')
  $source="$n\src\generated\GDGeometry.cpp";Set-Content $source ('#include <Native/GpuResources.hpp>'+"`n"+$geometry)
 }
 if($unit -eq 'GDTexture'){
  $texture=Get-Content -Raw $source
  if(!$texture.Contains('OSCachedToPhysical(image_ptr)')){throw 'Original texture address anchor changed'}
  $texture=$texture.Replace('OSCachedToPhysical(image_ptr)','Native::gpuResources().byteAddress(image_ptr)').Replace('{0, 4, 1, 5, 2, 6.}','{0, 4, 1, 5, 2, 6}')
  $source="$n\src\generated\GDTexture.cpp";Set-Content $source ('#include <Native/GpuResources.hpp>'+"`n"+$texture)
 }
 Invoke-Expression $line
 if($LASTEXITCODE){Get-Content "$out\$unit.log" -Tail 15;exit 1}
}




