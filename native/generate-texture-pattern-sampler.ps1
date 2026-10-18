$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\J3DGraphAnimator\J3DAnimation.cpp" -Raw
$a=$s.IndexOf('J3DAnmTexPattern::J3DAnmTexPattern()');$b=$s.IndexOf('void J3DAnmVisibilityFull::getVisibility(', $a)
if($a -lt 0 -or $b -lt 0){throw 'Original texture pattern sampling missing'}
('#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>'+"`r`n"+$s.Substring($a,$b-$a))|Set-Content "$n\src\generated\TexturePatternSampler.cpp"
$a=$s.IndexOf('void J3DAnmTexPattern::searchUpdateMaterialID(J3DMaterialTable*');$b=$s.IndexOf('void J3DAnmTexPattern::searchUpdateMaterialID(J3DModelData*', $a)
if($a -lt 0 -or $b -lt 0){throw 'Original texture material matching missing'}
$body=$s.Substring($a,$b-$a)
$mat=Get-Content "$p\Petari\src\JSystem\J3DGraphAnimator\J3DMaterialAttach.cpp" -Raw
$a=$mat.IndexOf('void J3DMaterialTable::clear()');$b=$mat.IndexOf('int J3DMaterialTable::removeMatColorAnimator(', $a)
('#include <JSystem/J3DGraphAnimator/J3DMaterialAttach.hpp>'+"`r`n"+'#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>'+"`r`n"+$mat.Substring($a,$b-$a)+$body)|Set-Content "$n\src\generated\TexturePatternMaterialMatch.cpp"
