$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
function Extract([string]$source,[string]$signature){
 $start=$source.IndexOf($signature);if($start -lt 0){throw "Original blend anchor missing: $signature"};$brace=$source.IndexOf('{',$start);$depth=1;$end=$brace+1
 while($depth){if($source[$end] -eq '{'){$depth++}elseif($source[$end] -eq '}'){$depth--};$end++};$source.Substring($start,$end-$start)
}
$s=Get-Content "$p\Petari\src\Game\Util\MathUtil.cpp" -Raw
$parts=@('#include <revolution/gx/GXLighting.h>','#include <Game/Util/MathUtil.hpp>','namespace MR {',(Extract $s '    void blendColor('),(Extract $s '    void blendVec('),'}')
Set-Content "$n\src\generated\LightBlendKernel.cpp" ($parts -join [Environment]::NewLine)
$parts=@('#include <revolution/types.h>','namespace MR {',(Extract $s '    f32 getInterpolateValue('),(Extract $s '    f32 getLinerValue('),'}')
Set-Content "$n\src\generated\LightBlendScalarFixture.cpp" ($parts -join [Environment]::NewLine)
$s=Get-Content "$p\Petari\src\Game\Map\LightPointCtrl.cpp" -Raw
$parts=@('#include <Game/Map/LightPointCtrl.hpp>','#include <Game/Util/MathUtil.hpp>','namespace {constexpr s32 sDefaultBlendTime=30;constexpr f32 sDefaultDistRef=15;}',(Extract $s 'LightPointCtrl::LightPointCtrl()'),(Extract $s 'void LightPointCtrl::clearPointLight('),(Extract $s 'void LightPointCtrl::blendPointLight('))
Set-Content "$n\src\generated\PointLightBlendFixtureKernel.cpp" ($parts -join [Environment]::NewLine)
