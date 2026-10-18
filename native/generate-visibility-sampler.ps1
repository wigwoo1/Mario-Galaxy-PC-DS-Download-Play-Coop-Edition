$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\J3DGraphAnimator\J3DAnimation.cpp" -Raw
$a=$s.IndexOf('void J3DAnmVisibilityFull::getVisibility(');$b=$s.IndexOf('void J3DAnmTexPattern::searchUpdateMaterialID(', $a)
if($a -lt 0 -or $b -lt 0){throw 'Original visibility sampler missing'}
('#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>'+"`r`n"+$s.Substring($a,$b-$a))|Set-Content "$n\src\generated\VisibilitySampler.cpp"
