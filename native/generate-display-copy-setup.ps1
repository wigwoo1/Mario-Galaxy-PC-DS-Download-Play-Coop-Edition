$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$s=Get-Content -Raw "$p\Petari\src\RVL_SDK\gx\GXFrameBuf.c"
$body=''
foreach($signature in @('static u32 __GXGetNumXfbLines(','u32 GXSetDispCopyYScale(','void GXSetCopyClamp(','void GXSetDispCopySrc(','void GXSetDispCopyDst(','void GXSetDispCopyFrame2Field(','void GXSetDispCopyGamma(')){
 $start=$s.IndexOf($signature);if($start -lt 0){throw 'Original display scaling anchor missing'};$brace=$s.IndexOf('{',$start);$depth=1;$end=$brace+1;while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++};$body+=$s.Substring($start,$end-$start)+"`n"
}
$body=@'
#undef gx
#include <Native/DisplayCopyState.hpp>
#include <revolution/gd/GDBase.h>
#include <cmath>
#include <stdexcept>
namespace Native {void resetTextureCopyInterlacing();void setTextureCopyClamp(bool,bool);}
namespace {Native::DisplayCopyState copyState;auto* gx=&copyState;}
namespace Native {const DisplayCopyState& displayCopyState(){return copyState;}}

#include <private/gen_reg.h>
#include <private/pe_reg.h>
#define FAST_GPFLAGSET(reg,value,name) ((reg)=((reg)&~u32(name##_MASK))|((u32(value)<<name##_SHIFT)&u32(name##_MASK)))
#include <revolution/gx/shortcut_pe_reg.h>
#include <revolution/gx/shortcut_gen_reg.h>
'@+"
"+$body
$body=$body.Replace('FALSE','false').Replace('GX_WRITE_RA_REG(','GDWriteBPCmd(').Replace('BP_SENT();','')
$body=$body.Replace('    iScale = (u32)(256.0f / vscale) & 0x1ff;','    if(!std::isfinite(vscale)||vscale<=0 || 256.0f/vscale>4294967040.0f || ((u32)(256.0f/vscale)&0x1ff)==0)throw std::invalid_argument("Invalid display-copy scale"); GDOverflowCheck(5);'+[Environment]::NewLine+'    iScale = (u32)(256.0f / vscale) & 0x1ff;')
$body=$body.Replace('    SC_PE_COPY_CMD_SET_CLAMP_TOP(gx->cpTex, clmpT);','    Native::setTextureCopyClamp(clmpT,clmpB);').Replace('    SC_PE_COPY_CMD_SET_CLAMP_BOTTOM(gx->cpTex, clmpB);','')
$body=$body.Replace('SC_PE_COPY_CMD_SET_INTERLACED(gx->cpTex, GX_COPY_PROGRESSIVE);','Native::resetTextureCopyInterlacing();')
Set-Content "$n\src\generated\DisplayCopySetupKernel.cpp" ('#include <revolution/gx.h>'+"`n"+$body)










