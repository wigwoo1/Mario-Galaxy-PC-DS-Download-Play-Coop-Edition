$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$s=Get-Content "$p\Petari\src\RVL_SDK\gx\GXLight.c" -Raw
$body=''
foreach($signature in @('void GXSetChanAmbColor(','void GXSetChanMatColor(')){
 $start=$s.IndexOf($signature);if($start -lt 0){throw 'Original color setup anchor missing'};$brace=$s.IndexOf('{',$start);$depth=1;$end=$brace+1;while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++};$body+=$s.Substring($start,$end-$start)+[Environment]::NewLine
}
$body=$body.Replace('(*(u32*)(&amb_color))','pack(amb_color)').Replace('*(u32*)(&amb_color)','pack(amb_color)').Replace('(*(u32*)(&mat_color))','pack(mat_color)').Replace('*(u32*)(&mat_color)','pack(mat_color)')
$body=$body.Replace('gx->dirtyState |= (0x100 << colIdx);','GDOverflowCheck(9);GDWriteXFCmd(0x100a+colIdx,reg);').Replace('gx->dirtyState |= (0x400 << colIdx);','GDOverflowCheck(9);GDWriteXFCmd(0x100c+colIdx,reg);')
$body=$body.Replace('GXSetChanAmbColor','OriginalGXSetChanAmbColor').Replace('GXSetChanMatColor','OriginalGXSetChanMatColor')
$prefix=@'
#include <revolution/gx/GXLighting.h>
#include <revolution/gd/GDBase.h>
#include <private/xf_mem.h>
#include <revolution/gx/GXRegs.h>
#undef FAST_GPFLAGSET
#define FAST_GPFLAGSET(reg,value,name) ((reg)=((reg)&~u32(name##_MASK))|((u32(value)<<name##_SHIFT)&u32(name##_MASK)))
#include <revolution/gx/shortcut_xf_mem.h>
#undef gx
namespace {struct ColorCache{u32 ambColor[2]{},matColor[2]{};};ColorCache colors;auto* gx=&colors;
u32 pack(GXColor c){return (u32(c.r)<<24)|(u32(c.g)<<16)|(u32(c.b)<<8)|c.a;}}
'@
Set-Content "$n\src\generated\LightingColorKernel.cpp" ($prefix+[Environment]::NewLine+$body)



