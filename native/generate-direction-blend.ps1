$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\MathUtil.cpp" -Raw
function ExtractBlend([string]$sig){$a=$s.IndexOf($sig);if($a -lt 0){throw "Missing blend anchor: $sig"};$b=$s.IndexOf('{',$a);$d=1;$e=$b+1;while($d){if($s[$e] -eq '{'){$d++}elseif($s[$e] -eq '}'){$d--};$e++};$s.Substring($a,$e-$a)}
$body=@('#include <Game/Util/MathUtil.hpp>','#include <JSystem/JMath/JMATrigonometric.hpp>','namespace { static f32* gAcosTable; }','namespace MR {')
foreach($sig in @('    void initAcosTable(','    f32 acosEx(','    f32 diffAngleAbs(const TVec3f&','    f32 diffAngleSigned(const TVec3f&','    void vecBlendNormal(','    bool vecBlendSphere(')){$body+=((ExtractBlend $sig).Replace('PSVECAdd(vA, vB, pVec);','pVec->add(vA, vB);'))}
$body+='}'
Set-Content "$n\src\generated\DirectionBlendKernel.cpp" ($body -join [Environment]::NewLine)



$body=@('#include <Game/Util/MathUtil.hpp>','#include <Game/Util/MtxUtil.hpp>','#include <JSystem/JGeometry/TUtil.hpp>','namespace MR {')
foreach($sig in @('    bool makeAxisAndCosignVecToVec(', '    f32 turnVecToPlane(', '    bool turnVecToVecCosOnPlane(TVec3f* pDst,', '    bool turnVecToVecCosOnPlane(TVec3f* pVec,', '    void rotateQuatRollBall(', '    void addRandomVector(','    void turnRandomVector(','    bool turnQuat(','    bool turnQuatXDirRad(')){$body+=(ExtractBlend $sig)}
$s=Get-Content "$p\Petari\src\Game\Util\MtxUtil.cpp" -Raw
foreach($sig in @('    void turnMtxToXDirDegree(','    void turnMtxToLocalDirDegree(')){$body+=(ExtractBlend $sig)}
$body+='}'
Set-Content "$n\src\generated\DirectionTurnKernel.cpp" ($body -join [Environment]::NewLine)



