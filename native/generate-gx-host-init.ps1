$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=Get-Content "$p\Petari\src\RVL_SDK\gx\GXInit.c" -Raw
function Extract-Declaration([string]$text,[string]$anchor){
 $start=$text.IndexOf($anchor,[StringComparison]::Ordinal)
 if($start -lt 0){throw "Missing GX initializer declaration: $anchor"}
 $end=$text.IndexOf('};',$start,[StringComparison]::Ordinal)
 if($end -lt 0){throw "Unterminated GX initializer declaration: $anchor"}
 return $text.Substring($start,$end+2-$start)
}
function Extract-Function([string]$text,[string]$anchor){
 $start=$text.IndexOf($anchor,[StringComparison]::Ordinal)
 if($start -lt 0){throw "Missing GX initializer function: $anchor"}
 $open=$text.IndexOf('{',$start)
 if($open -lt 0){throw "Missing function body: $anchor"}
 $depth=0
 for($i=$open;$i -lt $text.Length;$i++){
  if($text[$i] -eq '{'){$depth++}
  elseif($text[$i] -eq '}'){$depth--;if($depth -eq 0){return $text.Substring($start,$i+1-$start)}}
 }
 throw "Unterminated function: $anchor"
}
$vat=Extract-Declaration $source 'static GXVtxAttrFmtList GXDefaultVATList[]'
$projection=Extract-Declaration $source 'static f32 GXDefaultProjData[]'
$white=Extract-Declaration $source 'static u16 DefaultTexData[]'
$regionAddress=Extract-Declaration $source 'static u32 GXTexRegionAddrTable[]'
$regionCallback=Extract-Function $source 'static GXTexRegion* __GXDefaultTexRegionCallback(const GXTexObj* t_obj, GXTexMapID id) {'
$tlutCallback=Extract-Function $source 'static GXTlutRegion* __GXDefaultTlutRegionCallback(u32 idx) {'
$initGX=Extract-Function $source 'void __GXInitGX()'
$copyClear=$initGX.IndexOf('    GXSetCopyClear', [StringComparison]::Ordinal)
if($copyClear -lt 0){throw 'Missing original GX default-state setup anchor'}
$initGX='void __GXInitGX() {'+"`n"+'    GXRenderModeObj* rmode = &nativeRenderMode;'+"`n"+'    GXTexObj tex_obj;'+"`n"+'    Mtx identity_mtx;'+"`n"+'    GXColor clear = {64, 64, 64, 255};'+"`n"+'    GXColor black = {0, 0, 0, 0};'+"`n"+'    GXColor white = {255, 255, 255, 255};'+"`n"+'    u32 i;'+"`n`n"+$initGX.Substring($copyClear)
$stateStart=$source.IndexOf('    gx->genMode = 0;', $source.IndexOf('GXFifoObj* GXInit(void* base, u32 size)',[StringComparison]::Ordinal), [StringComparison]::Ordinal)
$stateEnd=$source.IndexOf('    {`n        u32 reg;', $stateStart, [StringComparison]::Ordinal)
if($stateEnd -lt 0){$stateEnd=$source.IndexOf("    {`r`n        u32 reg;",$stateStart,[StringComparison]::Ordinal)}
if($stateStart -lt 0 -or $stateEnd -lt 0){throw 'Missing original GX cached-state initialization block'}
$state=$source.Substring($stateStart,$stateEnd-$stateStart)
$prefix=@'
#include <revolution/gx.h>
#include <revolution/gx/GXTypes.h>
#include <revolution/gx/GXRegs.h>
#include <revolution/gd/GDBase.h>
#include <revolution/mtx.h>
#include <Native/GpuResources.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

#define GX_TMEM_LO 0x00000
#define GX_TMEM_HI 0x80000
#define GX_32k 0x08000
#define GX_8k 0x02000

namespace {
GXData hostGXData{};
GXFifoObj hostFifo{};
GDLObj hostCommandList{};
__GXFifoObj hostFifoState{};
bool hostInitialized=false;
bool whiteTextureRegistered=false;
GXRenderModeObj nativeRenderMode=[] { GXRenderModeObj mode{}; mode.viTVmode=VI_TVMODE_NTSC_INT; mode.fbWidth=640; mode.efbHeight=480; mode.xfbHeight=480; mode.viWidth=640; mode.viHeight=480; mode.xFBmode=VI_XFBMODE_SF; return mode; }();
}
GXData* const __GXData=&hostGXData;

'@
$hostInitPrefix=@'
extern "C" GXFifoObj* GXInit(void* base,u32 size){
    if(!base||size<4096||(reinterpret_cast<std::uintptr_t>(base)&31u)||reinterpret_cast<std::uintptr_t>(base)>std::numeric_limits<std::uintptr_t>::max()-size)return nullptr;
    auto* previous=__GDCurrentDL;
    std::memset(&hostGXData,0,sizeof(hostGXData));
    hostGXData.inDispList=GX_FALSE;hostGXData.dlSaveContext=GX_TRUE;hostGXData.abtWaitPECopy=GX_TRUE;
    hostGXData.tcsManEnab=0;hostGXData.tevTcEnab=0;
    GXSetMisc(GX_MT_XF_FLUSH,GX_XF_FLUSH_NONE);
    u32 i=0;
'@
$cacheAndState=@'
    for(i=0;i<8;++i){
        GXInitTexCacheRegion(&hostGXData.TexRegions0[i],GX_FALSE,GXTexRegionAddrTable[i],GX_TEXCACHE_32K,GXTexRegionAddrTable[i+8],GX_TEXCACHE_32K);
        GXInitTexCacheRegion(&hostGXData.TexRegions1[i],GX_FALSE,GXTexRegionAddrTable[i+16],GX_TEXCACHE_32K,GXTexRegionAddrTable[i+24],GX_TEXCACHE_32K);
        GXInitTexCacheRegion(&hostGXData.TexRegions2[i],GX_TRUE,GXTexRegionAddrTable[i+32],GX_TEXCACHE_32K,GXTexRegionAddrTable[i+40],GX_TEXCACHE_32K);
    }
    for(i=0;i<16;++i)GXInitTlutRegion(&hostGXData.TlutRegions[i],0x80000+0x8000*8+0x2000*i,GX_TLUT_256);
    for(i=0;i<4;++i)GXInitTlutRegion(&hostGXData.TlutRegions[i+16],0x80000+0x8000*8+0x2000*16+0x8000*i,GX_TLUT_1K);
    try {
        if(!whiteTextureRegistered){Native::gpuResources().addNativeWords(DefaultTexData,sizeof(DefaultTexData));whiteTextureRegistered=true;}
        GDInitGDLObj(&hostCommandList,static_cast<u8*>(base),size);
        GDSetCurrent(&hostCommandList);
        hostFifoState={};hostFifoState.base=static_cast<u8*>(base);hostFifoState.top=static_cast<u8*>(base)+size;hostFifoState.size=size;
        hostFifoState.hiWatermark=size-16*1024;hostFifoState.loWatermark=size/2&~31u;
        hostFifoState.rdPtr=base;hostFifoState.wrPtr=base;hostFifoState.bind_cpu=GX_TRUE;hostFifoState.bind_gp=GX_TRUE;
        static_assert(sizeof(hostFifoState)<=sizeof(hostFifo),"Native FIFO state must fit the SDK opaque FIFO object");
        std::memcpy(&hostFifo,&hostFifoState,sizeof(hostFifoState));
        __GXInitGX();
        hostInitialized=true;
        return &hostFifo;
    } catch(...) {
        GDSetCurrent(previous);
        hostInitialized=false;
        return nullptr;
    }
}
'@
$hostApi=@'
extern "C" GXBool GXGetCPUFifo(GXFifoObj* output){
    if(!hostInitialized||!output)return GX_FALSE;
    std::memcpy(output,&hostFifo,sizeof(hostFifo));return GX_TRUE;
}
extern "C" GXBool GXGetGPFifo(GXFifoObj* output){return GXGetCPUFifo(output);}
extern "C" void GXGetFifoPtrs(const GXFifoObj* fifo,void** readPtr,void** writePtr){
    if(!fifo||!readPtr||!writePtr)return;
    __GXFifoObj state{};std::memcpy(&state,fifo,sizeof(state));
    *readPtr=state.rdPtr;*writePtr=hostInitialized&&__GDCurrentDL==&hostCommandList?__GDCurrentDL->ptr:state.wrPtr;
}
extern "C" void GXSetMisc(GXMiscToken token,u32 value){
    switch(token){
    case GX_MT_NULL:break;
    case GX_MT_XF_FLUSH:hostGXData.vNum=value;hostGXData.vNumNot=!hostGXData.vNum;hostGXData.bpSentNot=GX_TRUE;if(value)hostGXData.dirtyState|=8;break;
    case GX_MT_DL_SAVE_CONTEXT:hostGXData.dlSaveContext=value!=0;break;
    case GX_MT_ABORT_WAIT_COPYOUT:hostGXData.abtWaitPECopy=value!=0;break;
    }
}
'@
$contents=$prefix+$vat+"`n"+$projection+"`n"+$white+"`n"+$regionAddress+"`n"+$regionCallback+"`n"+$tlutCallback+"`n"+$initGX+"`n"+$hostInitPrefix+$state+"`n"+$cacheAndState+$hostApi
$out="$n\src\generated\GxHostInitKernel.cpp"
Set-Content -LiteralPath $out -Value $contents -Encoding utf8
Write-Output "Generated $out from Petari GXInit defaults; MMIO FIFO registration omitted."
