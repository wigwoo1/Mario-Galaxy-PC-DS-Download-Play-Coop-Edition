$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\RVL_SDK\gx\GXLight.c" -Raw
$end=$s.IndexOf('static inline u32 ConvLightID2Num')
if($end -lt 0){throw 'Original light initialization boundary missing'}
$s=$s.Substring(0,$end).Replace('revolution/gx.h','revolution/gx/GXLighting.h')
$s=$s.Replace('obj->Color = *(u32*)(&color);','obj->Color = (u32(color.r)<<24)|(u32(color.g)<<16)|(u32(color.b)<<8)|color.a;')
Set-Content "$n\src\generated\LightObjectKernel.cpp" ('#include <revolution/gx/GXTypes.h>'+[Environment]::NewLine+$s)



