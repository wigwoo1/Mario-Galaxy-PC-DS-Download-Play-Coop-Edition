$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$root=Split-Path $n -Parent
function Replace-CameraOnce([string]$text,[string]$before,[string]$after){
 if([regex]::Matches($text,[regex]::Escape($before)).Count-ne 1){throw 'Camera animation source anchor missing or duplicated'}
 return $text.Replace($before,$after)
}
# Preserve every member's type/size/order; replace only the unused raw resource
# pointer with an owning pointer. Existing camera parameter handles stay unchanged.
$header=[IO.File]::ReadAllText((Join-Path $root 'Petari\include\Game\Camera\CameraAnim.hpp'))
$header=Replace-CameraOnce $header 'class CameraAnim : public Camera {' ("namespace Native { class OwnedCameraAnimation; }`nclass CameraAnim : public Camera {")
$header=Replace-CameraOnce $header '    virtual void reset();' ("    ~CameraAnim() override;`n    virtual void reset();")
$header=Replace-CameraOnce $header 'u8* mFileData;' 'Native::OwnedCameraAnimation* mNativeAnimation;'
$include=Join-Path $n 'include-runtime\Game\Camera'
New-Item -ItemType Directory -Force -Path $include | Out-Null
[IO.File]::WriteAllText((Join-Path $include 'CameraAnim.hpp'),$header,[Text.UTF8Encoding]::new($false))
$source=[IO.File]::ReadAllText((Join-Path $root 'Petari\src\Game\Camera\CameraAnim.cpp'))
foreach($stride in @(3,4)){
 $anchor="f32 KeyCamAnmDataAccessor::get${stride}f(f32 key, u32 offset, u32 count) const {"
 $guard="`n    if (key <= mValues[offset]) return mValues[offset + 1];`n    const u32 last = offset + (count - 1) * $stride;`n    if (key >= mValues[last]) return mValues[last + 1];"
 $source=Replace-CameraOnce $source $anchor ($anchor+$guard)
}
$source=Replace-CameraOnce $source 'mFileData(), mIsPaused()' 'mNativeAnimation(new Native::OwnedCameraAnimation()), mIsPaused()'
$source=Replace-CameraOnce $source 'void CameraAnim::reset() {' ("CameraAnim::~CameraAnim() { delete mNativeAnimation; delete mDataAccessor; delete mKeyDataAccessor; }`n`nvoid CameraAnim::reset() {")
$source=Replace-CameraOnce $source '    loadBin(pFile);' '    if (!loadBin(pFile)) return;'
$source=Replace-CameraOnce $source '    mFileData = pFile;' ''
$source=Replace-CameraOnce $source '    TPos3f mtx;' ("    if (!mNrFrames || !mFileDataAccessor) return nullptr;`n    TPos3f mtx;")
$start=$source.IndexOf('u32 CameraAnim::getAnimFrame(u8* pFile) {')
$end=$source.IndexOf('CamTranslatorBase* CameraAnim::createTranslator()', $start)
if($start-lt 0-or $end-le $start){throw 'Camera frame query boundaries changed'}
$query=@'
u32 CameraAnim::getAnimFrame(u8* pFile) {
    Native::CameraAnimationData decoded;
    return Native::decodeMountedCameraAnimation(decoded,pFile) ? decoded.frames : 0;
}

'@
$source=$source.Substring(0,$start)+$query+"`n"+$source.Substring($end)
$start=$source.IndexOf('bool CameraAnim::loadBin(u8* pFile) {')
if($start-lt 0){throw 'Camera animation load boundary changed'}
$load=@'
bool CameraAnim::loadBin(u8* pFile) {
    Native::CameraAnimationData decoded;
    if (!Native::decodeMountedCameraAnimation(decoded,pFile)) {
#ifdef GALAXY_DIAGNOSTIC_CAMERA_ANIMATION
        galaxyNativeBootTrace("Camera animation rejected unowned or invalid resource");
#endif
        return false;
    }
    if (!mNativeAnimation->loadDecoded(std::move(decoded))) return false;
    mFileDataAccessor=mNativeAnimation->bind(*mDataAccessor,*mKeyDataAccessor);
    mIsKey=mNativeAnimation->keyed();
    mNrFrames=mNativeAnimation->frames();
    mNrValues=mNativeAnimation->valueCount();
    const auto& metadata=mNativeAnimation->metadata();
    std::memcpy(&_4C,&metadata[1],4);
    std::memcpy(&_50,&metadata[2],4);
#ifdef GALAXY_DIAGNOSTIC_CAMERA_ANIMATION
    char line[200];std::snprintf(line,sizeof(line),"Camera animation loaded resource=%p frames=%u keyed=%u values=%u",static_cast<void*>(pFile),unsigned(mNrFrames),unsigned(mIsKey),unsigned(mNrValues));
    galaxyNativeBootTrace(line);
#endif
    return true;
}
'@
$source=$source.Substring(0,$start)+$load+"`n"
$source='#include <Native/OwnedCameraAnimation.hpp>'+"`n"+'#include <Native/CameraAnimationArchive.hpp>'+"`n"+'#include <JSystem/JMath/JMath.hpp>'+"`n"+$source
$source='#include <cstdio>'+"`n"+'#ifdef GALAXY_DIAGNOSTIC_CAMERA_ANIMATION'+"`n"+'extern "C" void galaxyNativeBootTrace(const char*);'+"`n"+'#endif'+"`n"+$source
[IO.File]::WriteAllText((Join-Path $n 'src\generated\CameraAnimRuntime.cpp'),$source,[Text.UTF8Encoding]::new($false))
