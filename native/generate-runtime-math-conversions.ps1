$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\MathUtil.cpp" -Raw
$out="#include `"Game/Util/MathUtil.hpp`"`r`n#include <JSystem/JGeometry/TUtil.hpp>`r`n#include <JSystem/JMath/JMATrigonometric.hpp>`r`n#include `"Game/Util/MtxUtil.hpp`"`r`n#include <cmath>`r`n#include <revolution/mtx.h>`r`nnamespace MR {`r`n"
$constant='const Vec gZeroVec = {0.0f, 0.0f, 0.0f};'
if(!$s.Contains($constant)){throw 'Original zero vector definition missing'}
$out=$out.Replace("namespace MR {`r`n",$constant+"`r`nnamespace MR {`r`n")
foreach($sig in @('    bool turnQuatYDirRad(', '    bool turnQuatZDirRad(', '    void clampVecAngleDeg(', '    void normalize(TPos3f*', '    void makeQuatRotateRadian(', '    bool isOppositeDirection(', '    f32 diffAngleSignedHorizontal(', '    void createBoundingBox(', '    f32 getLinerValueFromMinMax(f32 x, f32 min, f32 max, f32 start, f32 end)', '    bool turnVecToVecCos(TVec3f* pDst, const TVec3f& rFrom, const TVec3f& rTo, f32 cosAngle, const TVec3f& rAxis, f32 fallbackLength)', '    u32 getMinAbsElementIndex(const TVec3f& rVec)', '    f32 sign(f32 x)', '    bool isNan(const TVec3f& rVec)', '    f32 getFootPoint(const TVec3f& rPointA, const TVec3f& rPointB, const TVec3f& rPos, TVec3f* pDst)', '    bool normalizeOrZero(const TVec3f& rSrc, TVec3f* pDst)', '    u8 lerp(u8 start, u8 end, f32 t)', '    void fixed16ToFloat(TVec3f* pDst, const TVec3s& rPSrc, u8 q)', '    void floatToFixed16(TVec3s* pDst, const TVec3f& rPSrc, u8 q)', '    GXColor lerp(GXColor start, GXColor end, f32 t)', '    void getRotatedAxisY(TVec3f* pDst, const TVec3f& rPSrc)', '    void getRotatedAxisZ(TVec3f* pDst, const TVec3f& rPSrc)', '    bool calcReboundVelocity(TVec3f* pVel, const TVec3f& rNormal, f32 reboundRateV, f32 reboundRateH)', '    bool calcReboundVelocity(TVec3f* pVel, const TVec3f& rNormal, f32 reboundRate)', '    bool calcReflectionVector(TVec3f* pDir, const TVec3f& rNorm, f32 rate, f32 cosAngleMin)', '    void calcParabolicFunctionParam(f32* pAccel, f32* pVel, f32 max, f32 end)', '    void makeAxisVerticalZX(TVec3f* pVec, const TVec3f& rAxis)', '    void rotateVecDegree(TVec3f* pDst, const TVec3f& rSrc, const TVec3f& rAxis, f32 degree)', '    void rotateVecDegree(TVec3f* pDst, const TVec3f& rAxis, f32 degree)', '    f32 getMaxAbsElement(const TVec3f& rVec)', '    void makeAxisFrontUp(TVec3f* pSide, TVec3f* pUp, const TVec3f& rFront, const TVec3f& rUp)', '    void makeAxisFrontSide(TVec3f* pUp, TVec3f* pSide, const TVec3f& rFront, const TVec3f& rSide)', '    f32 calcPerpendicFootToLine(TVec3f* pDst, const TVec3f& rPos, const TVec3f& rPointA, const TVec3f& rPointB)', '    f32 calcPerpendicFootToLineInside(TVec3f* pDst, const TVec3f& rPos, const TVec3f& rPointA, const TVec3f& rPointB)', '    f32 getScaleWithReactionValueZeroToOne(f32 x, f32 freq, f32 amplitude)', '    void makeAxisUpFront(TVec3f* pSide, TVec3f* pFront, const TVec3f& rUp, const TVec3f& rFront)', '    void makeAxisCrossPlane(TVec3f* pVec, TVec3f* pCross, const TVec3f& rNorm)', '    void clampLength(TVec3f* pDst, const TVec3f& rSrc, f32 length)', '    u32 getMaxElementIndex(const TVec3f& rVec)', '    f32 getMaxElement(const TVec3f& rVec)', '    void calcMomentRollBall(TVec3f* pMoment, const TVec3f& rVel, const TVec3f& rUp, f32 radius)', '    f32 turnVecToVecRadian(TVec3f* pDst, const TVec3f& rFrom, const TVec3f& rTo, f32 angle, const TVec3f& rAxis)', '    f32 turnVecToVecDegree(TVec3f* pDst, const TVec3f& rFrom, const TVec3f& rTo, f32 angle, const TVec3f& rAxis)', '    f32 calcDistanceXY(const TVec3f& rPos1, const TVec3f& rPos2)', '    bool isNearAngleRadianHV(const TVec3f& rA, const TVec3f& rB, const TVec3f& rAxis, f32 angleH, f32 angleV)', '    bool isNearAngleDegreeHV(const TVec3f& rA, const TVec3f& rB, const TVec3f& rAxis, f32 angleH, f32 angleV)', '    f32 getConvergeVibrationValue(f32 x, f32 start, f32 end, f32 dampScale, f32 rate)', '    f32 getReduceVibrationValue(f32 x, f32 time, f32 base, f32 amplitude, f32 freq)', '    void getRandomVector(TVec3f* pDst, f32 range)', '    f32 calcRotateY(f32 x, f32 z)', '    bool isNearAngleRadian(const TVec3f& rA, const TVec3f& rB, f32 angle)', '    bool isNearAngleDegree(const TVec3f& rA, const TVec3f& rB, f32 angle)', '    void makeQuatRotateDegree(TQuat4f* pQuat, const TVec3f& rRot)', '    void rotateQuatMoment(TQuat4f* pQuat, const TVec3f& rMoment)', '    void blendQuatUpFront(TQuat4f* pDst, const TVec3f& rUp, const TVec3f& rFront, f32 upRate, f32 frontRate)', '    void blendQuatUpFront(TQuat4f* pDst, const TQuat4f& rSrc, const TVec3f& rUp, const TVec3f& rFront, f32 upRate, f32 frontRate)')) {
$a=$s.IndexOf($sig);if($a -lt 0){throw "Missing signature: $sig"}
$b=$s.IndexOf('{',$a);$depth=1;$i=$b+1
while($depth -gt 0 -and $i -lt $s.Length){if($s[$i] -eq '{'){$depth++};if($s[$i] -eq '}'){$depth--};$i++}
if($depth -ne 0){throw 'Unbalanced source body'}
$out+=$s.Substring($a,$i-$a)+"`r`n"
}
$out+="}`r`n"
$out=$out.Replace('pQuat->setEulerDegree(rRot.x, rRot.y, rRot.z);', 'pQuat->setEuler(rRot.x * PI_180, rRot.y * PI_180, rRot.z * PI_180);')
$out=$out.Replace('__fpclassifyf(rVec.x) == 1', 'std::isnan(rVec.x)').Replace('__fpclassifyf(rVec.y) == 1', 'std::isnan(rVec.y)').Replace('__fpclassifyf(rVec.z) == 1', 'std::isnan(rVec.z)')
$out=$out.Replace('PSMTXRotAxisRad(mtx, rAxis, 0.00017453293f);','{ Vec axis{rAxis.x,rAxis.y,rAxis.z}; PSMTXRotAxisRad(mtx.toMtxPtr(), &axis, 0.00017453293f); }').Replace('PSMTXMultVec(mtx, horizonA, horizonA);','{ Vec input{horizonA.x,horizonA.y,horizonA.z}, output; PSMTXMultVec(mtx.toMtxPtr(), &input, &output); horizonA.set(output.x,output.y,output.z); }')
$out=$out.Replace('pMtx->setXYZDir(x, y, z);', 'pMtx->mMtx[0][0]=x.x; pMtx->mMtx[1][0]=x.y; pMtx->mMtx[2][0]=x.z; pMtx->mMtx[0][1]=y.x; pMtx->mMtx[1][1]=y.y; pMtx->mMtx[2][1]=y.z; pMtx->mMtx[0][2]=z.x; pMtx->mMtx[1][2]=z.y; pMtx->mMtx[2][2]=z.z;')
Set-Content "$n\src\generated\RuntimeMathConversions.cpp" $out






























