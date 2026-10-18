$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\JSystem\J3DGraphAnimator\J3DAnimation.cpp" -Raw
$start=$s.IndexOf('inline f32 J3DHermiteInterpolation(__REGISTER');$end=$s.IndexOf('#endif',$start)
if($start -lt 0 -or $end -lt 0){throw 'Signed Hermite source anchor changed'}
$s=$s.Insert($end,"#else`r`n    return JMAHermiteInterpolation(pp1, *pp2, *pp3, *pp4, *pp5, *pp6, *pp7);`r`n")
$pattern='(?s)(pTransform->mRotation\.[xyz] = )([^;]+?)\s*<< mDecShift;'
if(([regex]::Matches($s,$pattern)).Count -ne 6){throw 'Original rotation shift anchors changed'}
$s=[regex]::Replace($s,$pattern,'$1static_cast<s16>(u16(u32(static_cast<s32>($2)) << mDecShift));')
Set-Content "$n\src\generated\J3DAnimation.cpp" $s -Encoding UTF8
$start=$s.IndexOf('inline f32 J3DHermiteInterpolation(f32');$end=$s.IndexOf('void J3DAnmTransformKey::calcTransform',$start)
if($start -lt 0 -or $end -lt 0){throw 'Original sampler source anchors changed'}
Set-Content "$n\src\generated\OriginalKeySampler.hpp" ('#pragma once'+"`r`n"+'#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>'+"`r`n"+'#include <JSystem/JMath/JMath.hpp>'+"`r`n"+$s.Substring($start,$end-$start)) -Encoding UTF8
$kernel='#include "OriginalKeySampler.hpp"'+"`r`n"
foreach($signature in @('J3DAnmTransform::J3DAnmTransform(', 'void J3DAnmTransformKey::calcTransform(', 'void J3DFrameCtrl::init(s16', 'void J3DFrameCtrl::update()')){
    $start=$s.IndexOf($signature);$open=$s.IndexOf('{',$start);$depth=1;$end=$open+1
    while($depth){if($s[$end] -eq '{'){$depth++};if($s[$end] -eq '}'){$depth--};$end++}
    $kernel+=$s.Substring($start,$end-$start)+"`r`n"
}
Set-Content "$n\src\generated\OriginalTransformSampler.cpp" $kernel -Encoding UTF8
