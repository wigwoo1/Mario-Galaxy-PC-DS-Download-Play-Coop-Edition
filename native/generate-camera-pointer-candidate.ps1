$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
# Stage the ABI-changing header separately until every actor object is rebuilt.
$out="$n\build\camera-pointer-candidate"
New-Item -ItemType Directory -Force "$out\include\Game\Camera","$out\src" | Out-Null
function Replace-Checked([string]$source,[string]$before,[string]$after) {
    if(!$source.Contains($before)){throw "Pinned camera anchor missing: $before"}
    return $source.Replace($before,$after)
}
$header=Get-Content "$p\Petari\include\Game\Camera\CameraParamChunk.hpp" -Raw
$header=Replace-Checked $header 'mNum2() {' 'mNum2(), mAnimData(nullptr) {'
$header=Replace-Checked $header '/* 0x38 */ CameraParamString mString;' "/* 0x38 */ CameraParamString mString;`n    // Native-only resource pointer; authored num1 remains signed 32-bit.`n    u8* mAnimData;"
Set-Content "$out\include\Game\Camera\CameraParamChunk.hpp" $header
$chunk=Get-Content "$p\Petari\src\Game\Camera\CameraParamChunk.cpp" -Raw
$chunk=Replace-Checked $chunk 'mString = rOther.mString;' "mString = rOther.mString;`n    mAnimData = rOther.mAnimData;"
$chunk=Replace-Checked $chunk 'pReader->getValueInt("num1", &mGeneralParam->mNum1);' "mGeneralParam->mAnimData = nullptr;`n    pReader->getValueInt(`"num1`", &mGeneralParam->mNum1);"
Set-Content "$out\src\CameraParamChunk.cpp" $chunk
$translator=Get-Content "$p\Petari\src\Game\Camera\CamTranslatorAnim.cpp" -Raw
$translator=Replace-Checked $translator 'reinterpret_cast< u8* >(pChunk->mGeneralParam->mNum1)' 'pChunk->mGeneralParam->mAnimData'
Set-Content "$out\src\CamTranslatorAnim.cpp" $translator
$game=Get-Content "$p\Petari\src\Game\Camera\CameraManGame.cpp" -Raw
$game=Replace-Checked $game 'chunk->mGeneralParam->mNum1 = reinterpret_cast< s32 >(data);' 'chunk->mGeneralParam->mAnimData = reinterpret_cast<u8*>(data);'
Set-Content "$out\src\CameraManGame.cpp" $game
$util=Get-Content "$p\Petari\src\Game\Util\CameraUtil.cpp" -Raw
$util=Replace-Checked $util 'chunk->mGeneralParam->mNum1 = reinterpret_cast< s32 >(pAnimData);' 'chunk->mGeneralParam->mAnimData = reinterpret_cast<u8*>(pAnimData);'
$util=Replace-Checked $util 'pScreenPos->scale(1.0f / pos.w, *pos.toTVec3());' 'pScreenPos->scale(1.0f / pos.w, TVec3f(pos.x, pos.y, pos.z));'
$util=Replace-Checked $util '(GXProjectionType) nullptr' 'GX_PERSPECTIVE'
Set-Content "$out\src\CameraUtil.cpp" $util
Write-Output 'Staged pointer-safe camera producer/copy/load/reset/consumer adaptation; runtime ABI not activated.'


$holder=Get-Content "$p\Petari\src\Game\Camera\CameraParamChunkHolder.cpp" -Raw
$holder=Replace-Checked $holder '#include <JSystem/JKernel/JKRHeap.hpp>' '#include <Native/HeapPlacement.hpp>'
Set-Content "$out\src\CameraParamChunkHolder.cpp" $holder
$anim=Get-Content "$p\Petari\src\Game\Camera\CameraAnim.cpp" -Raw
Set-Content "$out\src\CameraAnim.cpp" ('#include <JSystem/JMath/JMath.hpp>'+"`n"+$anim)
# Native key sampling must not underflow before the first key or read a pair past the final key.
$anim=Get-Content "$out\src\CameraAnim.cpp" -Raw
foreach($stride in @(3,4)) {
    $anchor="f32 KeyCamAnmDataAccessor::get${stride}f(f32 key, u32 offset, u32 count) const {"
    $guard="`n    if (key <= mValues[offset]) return mValues[offset + 1];`n    const u32 last = offset + (count - 1) * $stride;`n    if (key >= mValues[last]) return mValues[last + 1];"
    $anim=Replace-Checked $anim $anchor ($anchor+$guard)
}
Set-Content "$out\src\CameraAnim.cpp" $anim
# Frame count queries now validate actual mounted archive ownership and decode big-endian data.
$anim=Get-Content "$out\src\CameraAnim.cpp" -Raw
$start=$anim.IndexOf('u32 CameraAnim::getAnimFrame(u8* pFile) {')
$end=$anim.IndexOf('CamTranslatorBase* CameraAnim::createTranslator()', $start)
if($start -lt 0 -or $end -le $start){throw 'Missing frame count query anchors'}
$body=@"
u32 CameraAnim::getAnimFrame(u8* pFile) {
    Native::CameraAnimationData decoded;
    return Native::decodeMountedCameraAnimation(decoded,pFile) ? decoded.frames : 0;
}

"@
$anim=$anim.Substring(0,$start)+$body+$anim.Substring($end)
Set-Content "$out\src\CameraAnim.cpp" ('#include <Native/CameraAnimationArchive.hpp>'+"`n"+$anim)
# Replace unused raw mFileData slot with an owning pointer of identical native size.
$header=Get-Content "$p\Petari\include\Game\Camera\CameraAnim.hpp" -Raw
$header=Replace-Checked $header 'class CameraAnim : public Camera {' "namespace Native { class OwnedCameraAnimation; }`nclass CameraAnim : public Camera {"
$header=Replace-Checked $header '    virtual void reset();' "    ~CameraAnim() override;`n    virtual void reset();"
$header=Replace-Checked $header 'u8* mFileData;' 'Native::OwnedCameraAnimation* mNativeAnimation;'
Set-Content "$out\include\Game\Camera\CameraAnim.hpp" $header
$anim=Get-Content "$out\src\CameraAnim.cpp" -Raw
$anim=Replace-Checked $anim 'mFileData(), mIsPaused()' 'mNativeAnimation(new Native::OwnedCameraAnimation()), mIsPaused()'
$anim=Replace-Checked $anim 'void CameraAnim::reset() {' "CameraAnim::~CameraAnim() { delete mNativeAnimation; delete mDataAccessor; delete mKeyDataAccessor; }`n`nvoid CameraAnim::reset() {"
$anim=Replace-Checked $anim '    loadBin(pFile);' '    if (!loadBin(pFile)) return;'
$anim=Replace-Checked $anim '    mFileData = pFile;' ''
$anim=Replace-Checked $anim '    TPos3f mtx;' "    if (!mNrFrames || !mFileDataAccessor) return nullptr;`n    TPos3f mtx;"
$start=$anim.IndexOf('bool CameraAnim::loadBin(u8* pFile) {')
if($start -lt 0){throw 'Missing original loadBin anchor'}
$body=@"
bool CameraAnim::loadBin(u8* pFile) {
    Native::CameraAnimationData decoded;
    if (!Native::decodeMountedCameraAnimation(decoded,pFile)) return false;
    if (!mNativeAnimation->loadDecoded(std::move(decoded))) return false;
    mFileDataAccessor=mNativeAnimation->bind(*mDataAccessor,*mKeyDataAccessor);
    mIsKey=mNativeAnimation->keyed();
    mNrFrames=mNativeAnimation->frames();
    mNrValues=mNativeAnimation->valueCount();
    const auto& metadata=mNativeAnimation->metadata();
    std::memcpy(&_4C,&metadata[1],4);
    std::memcpy(&_50,&metadata[2],4);
    return true;
}
"@
$anim=$anim.Substring(0,$start)+$body
Set-Content "$out\src\CameraAnim.cpp" ('#include <Native/OwnedCameraAnimation.hpp>'+"`n"+$anim)
