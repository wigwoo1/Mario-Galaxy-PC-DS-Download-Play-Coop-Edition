$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$s=Get-Content -Raw "$p\Petari\src\Game\Util\MathUtil.cpp"
function Extract([string]$signature){$start=$s.IndexOf($signature);if($start -lt 0){throw 'Original projection anchor missing'};$brace=$s.IndexOf('{',$start);$depth=1;$end=$brace+1;while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++};return $s.Substring($start,$end-$start)}
$near=Extract '    bool isNearZero(const TVec3f& rVec, f32 tolerance)'
$near="#ifndef GALAXY_SHARED_RUNTIME_MATH`n"+$near+"`n#endif`n"
$wrapper=(Extract '    f32 vecKillElement(').Replace('PSVECKillElement(rSrc, rKillDir, pDst)','nativeKillElement(rSrc, rKillDir, pDst)')
Set-Content "$n\src\generated\VectorProjectionKernel.cpp" ('#include <Game/Util/MathUtil.hpp>'+"`n"+'#include <cmath>'+"`n"+'namespace MR {static f32 nativeKillElement(const TVec3f& source,const TVec3f& direction,TVec3f* output){const f32 x=source.x,y=source.y,z=source.z;f32 dot=x*direction.x+y*direction.y+z*direction.z;output->set(std::fma(-dot,direction.x,x),std::fma(-dot,direction.y,y),std::fma(-dot,direction.z,z));return dot;}'+"`n"+$near+"`n"+$wrapper+"`n}`n")

