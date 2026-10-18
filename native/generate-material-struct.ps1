$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\libs\JSystem\include\JSystem\J3DGraphBase\J3DStruct.hpp" -Raw
$start=$s.IndexOf('inline void operator=(J3DTextureSRTInfo const& other)')
if($start -lt 0){throw 'SRT copy anchor'}
$end=$s.IndexOf('#endif',$start)
$s=$s.Insert($end,"#else`n        mScaleX=other.mScaleX;mScaleY=other.mScaleY;mRotation=other.mRotation;`n        mTranslationX=other.mTranslationX;mTranslationY=other.mTranslationY;`n")
Set-Content "$n\include\JSystem\J3DGraphBase\J3DStruct.hpp" $s -Encoding UTF8
$material=Get-Content "$p\Petari\libs\JSystem\include\JSystem\J3DGraphBase\J3DMaterial.hpp" -Raw
$anchor='return reinterpret_cast< uintptr_t >(mMaterialAnm) < 0xC0000000 ? mMaterialAnm : nullptr;'
if(!$material.Contains($anchor)){throw 'Original Wii material animator address check missing'}
$material=$material.Replace($anchor,'return mMaterialAnm;')
Set-Content "$n\include\JSystem\J3DGraphBase\J3DMaterial.hpp" $material -Encoding UTF8
