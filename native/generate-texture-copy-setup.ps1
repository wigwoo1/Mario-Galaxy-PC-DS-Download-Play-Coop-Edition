$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$s=Get-Content -Raw "$p\Petari\src\RVL_SDK\gx\GXFrameBuf.c"
$body=''
foreach($signature in @('void GXSetTexCopySrc(','void GXSetTexCopyDst(')){
 $start=$s.IndexOf($signature);if($start -lt 0){throw 'Original display scaling anchor missing'};$brace=$s.IndexOf('{',$start);$depth=1;$end=$brace+1;while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++};$body+=$s.Substring($start,$end-$start)+"`n"
}
$texture=Get-Content -Raw "$p\Petari\src\RVL_SDK\gx\GXTexture.c"
foreach($signature in @('static void __GXGetTexTileShift(','void __GetImageTileCount(')){
$start=$texture.IndexOf($signature);if($start -lt 0){throw "Texture tile helper missing"};$brace=$texture.IndexOf('{' ,$start);$depth=1;$end=$brace+1;while($depth){if($texture[$end] -eq '{'){$depth++}elseif($texture[$end] -eq '}'){$depth--};$end++};$body=$texture.Substring($start,$end-$start)+"`n"+$body
}
$body=@'
#undef gx
#include <Native/TextureCopyState.hpp>
namespace {Native::TextureCopyState copyState;auto* gx=&copyState;}
namespace Native {const TextureCopyState& textureCopyState(){return copyState;} void resetTextureCopyInterlacing(){copyState.cpTex&=~(3u<<12);} void setTextureCopyClamp(bool top,bool bottom){copyState.cpTex=(copyState.cpTex&~3u)|u32(top)|(u32(bottom)<<1);}}
static void __GXGetTexTileShift(GXTexFmt,u32*,u32*);
#include <private/gen_reg.h>
#include <private/pe_reg.h>
#define FAST_GPFLAGSET(reg,value,name) ((reg)=((reg)&~u32(name##_MASK))|((u32(value)<<name##_SHIFT)&u32(name##_MASK)))
#include <revolution/gx/shortcut_pe_reg.h>
#include <revolution/gx/shortcut_gen_reg.h>
'@+"
"+$body
$body=$body.Replace('FALSE','false')
Set-Content "$n\src\generated\TextureCopySetupKernel.cpp" ('#include <revolution/gx.h>'+"`n"+$body)








