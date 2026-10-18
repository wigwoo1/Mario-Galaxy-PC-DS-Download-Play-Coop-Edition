$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\MtxUtil.cpp" -Raw
$out="#include `"Game/Util/MathUtil.hpp`"`r`n#include `"Game/Util/MtxUtil.hpp`"`r`n#include <JSystem/JMath/JMATrigonometric.hpp>`r`n#include <JSystem/JMath/JMath.hpp>`r`nstatic Mtx tmpmtx_ry = {{1.0f,0.0f,0.0f,0.0f},{0.0f,1.0f,0.0f,0.0f},{0.0f,0.0f,1.0f,0.0f}};`r`nnamespace MR {`r`n"
foreach($sig in @('    void rotateMtxLocalX(', '    void rotateMtxLocalY(', '    void rotateMtxLocalZ(', '    void rotateMtxLocalXDegree(', '    void rotateMtxLocalYDegree(', '    void rotateMtxLocalZDegree(', '    void orthogonalize(TPos3f* pMtx)', '    void rotateMtxMoment(TPos3f* pDst, const TPos3f& rSrc, const TVec3f& rMoment)', '    void rotateMtxMoment(TPos3f* pMtx, const TVec3f& rMoment)', '    void addTransMtxLocal(MtxPtr pMtx, const TVec3f& rVec)', '    void addTransMtxLocalX(MtxPtr pMtx, f32 x_coord)', '    void makeMtxFrontNoSupport(TPos3f* pDst, const TVec3f& rFront)', '    void makeMtxUpSide(TPos3f* pDst, const TVec3f& rUp, const TVec3f& rSide)', '    void preScaleMtx(MtxPtr pMtx, f32 scale)', '    void preScaleMtx(MtxPtr pMtx, const TVec3f& rScale)', '    void preScaleMtx(MtxPtr pMtx, f32 sx, f32 sy, f32 sz)', '    void flattenMtx(MtxPtr pDst, MtxPtr pSrc, const TVec3f& rNormal)', '    void flattenMtx(MtxPtr pMtx, const TVec3f& rNormal)', '    void makeMtxUpNoSupport(TPos3f* pDst, const TVec3f& rUp)', '    void addTransMtx(MtxPtr pMtx, const TVec3f& rVec)', '    void addTransMtxLocalY(MtxPtr pMtx, f32 y_coord)', '    void addTransMtxLocalZ(MtxPtr pMtx, f32 z_coord)', '    void blendMtx(MtxPtr pMtxA, MtxPtr pMtxB, f32 blend, MtxPtr pDst)', '    void blendMtxRotate(MtxPtr pMtxA, MtxPtr pMtxB, f32 blend, MtxPtr pDst)', '    MtxPtr tmpMtxRotYRad(f32 rad)', '    void makeRTFromMtxPtr(TVec3f* pOutTrans, TVec3f* pOutRot, MtxPtr pSrc, bool toDegree)', '    void makeMtxFrontUp(TPos3f* pDst, const TVec3f& rFront, const TVec3f& rUp)', '    void makeMtxFrontUpPos(TPos3f* pDst, const TVec3f& rFront, const TVec3f& rUp, const TVec3f& rPos)')) {
$a=$s.IndexOf($sig);if($a -lt 0){throw "Missing signature: $sig"}
$b=$s.IndexOf('{',$a);$depth=1;$i=$b+1
while($depth -gt 0 -and $i -lt $s.Length){if($s[$i] -eq '{'){$depth++};if($s[$i] -eq '}'){$depth--};$i++}
if($depth -ne 0){throw 'Unbalanced source body'}
$out+=$s.Substring($a,$i-$a)+"`r`n"
}
$out+="}`r`n"
Set-Content "$n\src\generated\RuntimeMatrixTransforms.cpp" $out









