$original=Join-Path $PSScriptRoot '..\Petari\src\Game\Map\KCollision.cpp'
$s=[IO.File]::ReadAllText($original).Replace("`r`n","`n")
$edits=@{
 "finish:`n    f32 distance = MR::sqrt(*pDist);"="finish: {`n    f32 distance = MR::sqrt(*pDist);"
 "success:`n    return true;`n}`n`nbool KCollisionServer::isNearParallelNormal"="}`nsuccess:`n    return true;`n}`n`nbool KCollisionServer::isNearParallelNormal"
 'PSVECSubtract(rOrigin, v0, rel);'='rel.sub(rOrigin, *v0);'
 'PSVECDotProduct(rel, faceNormal)'='rel.dot(*faceNormal)'
 'PSVECDotProduct(faceNormal, rDir)'='faceNormal->dot(rDir)'
}
foreach($key in $edits.Keys){if(([regex]::Matches($s,[regex]::Escape($key))).Count -ne 1){throw "Unexpected source match: $key"};$s=$s.Replace($key,$edits[$key])}

$header=[IO.File]::ReadAllText((Join-Path $PSScriptRoot '..\Petari\include\Game\Map\KCollision.hpp'))
$header="#include <memory>`n#include <vector>`n#include <cstdint>`nclass NativeKclStorage;`n"+$header
$anchor='    void setData(void*);'
if(-not $header.Contains($anchor)){throw 'KCL header anchor'}
$header=$header.Replace($anchor,$anchor+"`n    void setNativeData(const std::vector<std::uint8_t>&);`n    void updateNativeDynamicData(const std::vector<std::uint8_t>&);`n    KCollisionServer(const std::vector<std::uint8_t>&, JMapInfo*);`n    KCollisionServer(const KCollisionServer&)=delete;`n    KCollisionServer& operator=(const KCollisionServer&)=delete;")
$anchor='    /* 0x08 */ f32 mMaxVertexDistance;'
if(-not $header.Contains($anchor)){throw 'KCL ownership anchor'}
$header=$header.Replace($anchor,$anchor+"`n    std::shared_ptr<NativeKclStorage> mNativeStorage;`n    KCLFile mNativeFile{};")
$dir=Join-Path $PSScriptRoot 'include-runtime\Game\Map'
New-Item -ItemType Directory -Force $dir | Out-Null
[IO.File]::WriteAllText((Join-Path $dir 'KCollision.hpp'),$header)
$s="#include `"../native_kcl_storage.hpp`"`n"+$s
$pattern='void KCollisionServer::setData\(void\* pData\) \{.*?\n\}'
if(([regex]::Matches($s,$pattern,[Text.RegularExpressions.RegexOptions]::Singleline)).Count -ne 1){throw 'setData anchor'}
$replacement=@"
void KCollisionServer::setData(void*) {
    throw std::logic_error("Native KCL loading requires setNativeData with bounded bytes");
}
KCollisionServer::KCollisionServer(const std::vector<std::uint8_t>& bytes,JMapInfo* attributes):mFile(nullptr),mapInfo(attributes),mMaxVertexDistance(1.0f){setNativeData(bytes);}
void KCollisionServer::updateNativeDynamicData(const std::vector<std::uint8_t>& bytes) {
    if(!mNativeStorage) throw std::logic_error("Native dynamic KCL not initialized");
    mNativeStorage->updateDynamic(bytes);
    mNativeFile.mThickness=mNativeStorage->thickness;mNativeFile.mMin=mNativeStorage->minimum;
    mNativeFile.mXMask=mNativeStorage->masks[0];mNativeFile.mYMask=mNativeStorage->masks[1];mNativeFile.mZMask=mNativeStorage->masks[2];
    mNativeFile.mBlockWidthShift=mNativeStorage->widthShift;
}
void KCollisionServer::setNativeData(const std::vector<std::uint8_t>& bytes) {
    auto storage=std::make_shared<NativeKclStorage>(bytes);
    KCLFile view{};
    view.mPos=storage->positions.data();view.mNorms=storage->normals.data();view.mPrisms=storage->prisms.data();
    view.mThickness=storage->thickness;view.mMin=storage->minimum;
    view.mXMask=storage->masks[0];view.mYMask=storage->masks[1];view.mZMask=storage->masks[2];
    view.mBlockWidthShift=storage->widthShift;view.mBlockXShift=storage->xShift;view.mBlockXYShift=storage->xyShift;
    mNativeStorage=std::move(storage);mNativeFile=view;mFile=&mNativeFile;
}
"@
$s=[regex]::Replace($s,$pattern,$replacement,[Text.RegularExpressions.RegexOptions]::Singleline)
$old='    return (reinterpret_cast< u8* >(mFile->mOctree) - reinterpret_cast< u8* >(mFile->mPrisms + 1)) / sizeof(KC_PrismData);'
if(-not $s.Contains($old)){throw 'triangle count anchor'}
$s=$s.Replace($old,'    if(!mNativeStorage)throw std::logic_error("Native KCL data not loaded"); return static_cast<s32>(mNativeStorage->triangleCount());')
$pattern='s32\* KCollisionServer::searchBlock\(s32\* pShift, const u32& rX, const u32& rY, const u32& rZ\) const \{.*?\n\}'
if(([regex]::Matches($s,$pattern,[Text.RegularExpressions.RegexOptions]::Singleline)).Count -ne 1){throw 'searchBlock anchor'}
$replacement=@"
s32* KCollisionServer::searchBlock(s32* pShift,const u32& x,const u32& y,const u32& z) const {
    if(!mNativeStorage)throw std::logic_error("Native KCL data not loaded");
    auto leaf=mNativeStorage->octree.nativeLookup(x,y,z,mFile->mBlockWidthShift,mFile->mBlockXShift,mFile->mBlockXYShift,mNativeStorage->triangleCount());
    *pShift=leaf.shift;return reinterpret_cast<s32*>(const_cast<u16*>(leaf.list));
}
"@
$s=[regex]::Replace($s,$pattern,$replacement,[Text.RegularExpressions.RegexOptions]::Singleline)
[IO.File]::WriteAllText((Join-Path $PSScriptRoot 'src\generated\KCollision.cpp'),$s)
# Bounded KCL entry point retains original stage/zone/camera setup after data binding.
$h=[IO.File]::ReadAllText((Join-Path $PSScriptRoot '..\Petari\include\Game\Map\CollisionParts.hpp'))
$a='    void init(const TPos3f&, HitSensor*, const void*, const void*, s32, bool);'
if(-not $h.Contains($a)){throw 'CollisionParts init declaration'}
$h="#include <vector>`n#include <cstdint>`n"+$h.Replace($a,$a+"`n    void initNative(const TPos3f&, HitSensor*, const std::vector<std::uint8_t>&, const std::vector<std::uint8_t>&, s32, bool);")
[IO.File]::WriteAllText((Join-Path $PSScriptRoot 'include-runtime\Game\Map\CollisionParts.hpp'),$h)
$s=[IO.File]::ReadAllText((Join-Path $PSScriptRoot '..\Petari\src\Game\Map\CollisionParts.cpp'))
$a='void CollisionParts::init(const TPos3f& rMatrix, HitSensor* pHitSensor, const void* pKclData, const void* pMapInfo, s32 keeperIndex,'
$b=$s.IndexOf($a);if($b -lt 0){throw 'CollisionParts init implementation'}
$o=$s.IndexOf('{',$b);$depth=1;$e=$o+1
while($depth){if($s[$e] -eq '{'){$depth++};if($s[$e] -eq '}'){$depth--};$e++}
$method=$s.Substring($b,$e-$b).Replace('CollisionParts::init(', 'CollisionParts::initNative(').Replace('const void* pKclData','const std::vector<std::uint8_t>& pKclData').Replace('const void* pMapInfo','const std::vector<std::uint8_t>& pMapInfo')
$a='    mServer->init(const_cast< void* >(pKclData), pMapInfo);'
if(-not $method.Contains($a)){throw 'CollisionParts server data call'}
$method=$method.Replace($a,"    mServer->setNativeData(pKclData);`n    if (!pMapInfo.empty()) mServer->mapInfo->attachNative(pMapInfo);")
[IO.File]::WriteAllText((Join-Path $PSScriptRoot 'src\generated\CollisionParts.cpp'),$s+"`n"+$method)
