$ErrorActionPreference='Continue'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$out="$n\build\actor-runtime-probe";$env:ZIG_GLOBAL_CACHE_DIR="$p\tools\zig-cache"
$header=Get-Content -Raw "$p\Petari\libs\JSystem\include\JSystem\JUtility\JUTTexture.hpp"
if(!$header.Contains('    JUTTexture() {')){throw 'Original default texture constructor anchor missing'}
$header=$header.Replace('    JUTTexture() {','    JUTTexture():mObj{},mTIMG(nullptr),mImage(nullptr),mEmbPalette(nullptr),mPalette(nullptr),mWrapS(0),mWrapT(0),mMinType(0),mMagType(0),mMinLod(0),mMaxLod(0),mLodBias(0),mTlutName(0),mFlag(0),_3C(nullptr) {')
New-Item -ItemType Directory -Force "$n\include-runtime\JSystem\JUtility" | Out-Null
Set-Content "$n\include-runtime\JSystem\JUtility\JUTTexture.hpp" $header
$directorHeader=Get-Content -Raw "$p\Petari\include\Game\Screen\CaptureScreenDirector.hpp"
$directorHeader=$directorHeader.Replace('    CaptureScreenDirector();','    CaptureScreenDirector();'+"`n"+'    ~CaptureScreenDirector() override;')
New-Item -ItemType Directory -Force "$n\include-runtime\Game\Screen" | Out-Null
Set-Content "$n\include-runtime\Game\Screen\CaptureScreenDirector.hpp" $directorHeader
$unit='CaptureScreenDirector';$source="$n\src\generated\CaptureScreenDirector.cpp"
$original=Get-Content -Raw "$p\Petari\src\Game\Screen\CaptureScreenDirector.cpp"
$original=$original.Replace('    GXSetCopyFilter(GX_FALSE, pRenderModeObj->sample_pattern, GX_FALSE, pRenderModeObj->vfilter);','    GDOverflowCheck(100);'+"`n"+'    GXSetCopyFilter(GX_FALSE, pRenderModeObj->sample_pattern, GX_FALSE, pRenderModeObj->vfilter);')
$original=$original.Replace('    mTexture->capture(0, 0, GX_TF_RGB565, false, 0);','    try {mTexture->capture(0, 0, GX_TF_RGB565, false, 0);} catch (...) {GXSetCopyFilter(GX_FALSE,pRenderModeObj->sample_pattern,GX_TRUE,pRenderModeObj->vfilter);throw;}')
$original += "`nCaptureScreenDirector::~CaptureScreenDirector(){delete mTexture;}`n"
$original='#include <revolution/gd/GDBase.h>'+"`n"+$original
Set-Content $source ('#include <Native/HeapPlacement.hpp>'+"`n"+$original)
$line=Get-Content "$n\probe-actor-runtime.ps1" | Where-Object {$_ -like '& *zig.exe*'}
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$out\$unit.log" -Tail 22;exit 1}
$unit='JUTTexture';$source="$n\src\generated\JUTTexture.cpp"
$texture=Get-Content -Raw "$p\Petari\src\JSystem\JUtility\JUTTexture.cpp"
$texture=$texture.Replace('    mFlag = mFlag & 2 | 1;','    mFlag = 1;')
$texture=$texture.Replace('    _3C = texBuf;','    struct BufferDelete{bool registered=false;void operator()(u8* bytes)const{if(registered)Native::releaseHeapGpuRange(bytes);JKRHeap::free(bytes,JKRHeap::findFromRoot(bytes));}};'+"`n"+'    std::unique_ptr<u8,BufferDelete> allocation(reinterpret_cast<u8*>(texBuf));'+"`n"+'    std::memset(texBuf,0,sizeof(ResTIMG));'+"`n"+'    _3C = texBuf;')
# JUTTexture is not a JKRDisposer. Scene heap reset must still expire its GPU range.
$texture=$texture.Replace('    storeTIMG(texBuf, static_cast< u8 >(0));','    Native::bindHeapGpuRange(texBuf,bufSize+sizeof(ResTIMG));allocation.get_deleter().registered=true;'+"`n"+'    storeTIMG(texBuf, static_cast< u8 >(0));')
$texture=$texture.Replace('    DCFlushRange(mImage, bufSize);','    DCFlushRange(mImage, bufSize);'+"`n"+'    allocation.release();')
$texture=$texture.Replace('        delete[] _3C;','        Native::releaseHeapGpuRange(_3C);JKRHeap::free(_3C,JKRHeap::findFromRoot(_3C));')
Set-Content $source ('#include <memory>'+"`n"+'#include <cstring>'+"`n"+'#include <Native/GpuResources.hpp>'+"`n"+'#include <Native/HeapGpuRange.hpp>'+"`n"+'#include <Native/HeapPlacement.hpp>'+"`n"+$texture)
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$out\$unit.log" -Tail 22;exit 1}

$unit='JUTPalette';$source="$p\Petari\src\JSystem\JUtility\JUTPalette.cpp"
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$out\$unit.log" -Tail 22;exit 1}
$unit='JUTVideo';$source="$n\src\generated\JUTVideo.cpp"
$video=Get-Content -Raw "$p\Petari\src\JSystem\JUtility\JUTVideo.cpp"
$video=[regex]::Replace($video,'\bTRUE\b','1')
$video=[regex]::Replace($video,'\bFALSE\b','0')
Set-Content $source $video
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$out\$unit.log" -Tail 22;exit 1}
$unit='JUTXfb';$source="$p\Petari\src\JSystem\JUtility\JUTXfb.cpp"
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$out\$unit.log" -Tail 22;exit 1}
$unit='JUTDirectPrint';$source="$n\src\generated\JUTDirectPrint.cpp"
$direct=Get-Content -Raw "$p\Petari\src\JSystem\JUtility\JUTDirectPrint.cpp"
$direct=$direct.Replace('ARRAY_SIZEU(buffer)','sizeof(buffer)')
$direct=$direct.Replace('    u16 x = position_x;','    if (buffer_length >= static_cast<int>(sizeof(buffer))) buffer_length = sizeof(buffer)-1;'+"`n"+'    u16 x = position_x;')
Set-Content $source $direct
Invoke-Expression $line
if($LASTEXITCODE){Get-Content "$out\$unit.log" -Tail 22;exit 1}





