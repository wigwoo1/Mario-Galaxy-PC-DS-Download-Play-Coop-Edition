$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$s=Get-Content -Raw "$p\Petari\src\RVL_SDK\gx\GXPixel.c"
$body=''
foreach($signature in @('void GXSetFog(')){
 $start=$s.IndexOf($signature);if($start -lt 0){throw 'Original display scaling anchor missing'};$brace=$s.IndexOf('{',$start);$depth=1;$end=$brace+1;while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++};$body+=$s.Substring($start,$end-$start)+"`n"
}
$body=$body.Replace('GX_WRITE_RA_REG(','GDWriteBPCmd(').Replace('BP_SENT();','')
$body=$body.Replace('    u32 reg;','    GDOverflowCheck(15);'+"`n"+'    u32 reg;').Replace('    u32 msLoc[4];','    GDOverflowCheck(30);'+"`n"+'    u32 msLoc[4];')
$body=@'
#include <revolution/gd/GDBase.h>
#include <cstring>
#include <private/tev_reg.h>
#include <private/pe_reg.h>
#define FAST_GPFLAGSET(reg,value,name) ((reg)=((reg)&~u32(name##_MASK))|((u32(value)<<name##_SHIFT)&u32(name##_MASK)))
#include <revolution/gx/shortcut_pe_reg.h>
#include <revolution/gx/shortcut_tev_reg.h>
'@+"
"+$body
$body=$body.Replace('SC_TEV_FOG_COLOR_SET_RGB(fogclr, (rgba >> 8));','fogclr=(fogclr&0xff000000)|((rgba>>8)&0xffffff);')
$body=$body.Replace('    u32 fogclr =','    GDOverflowCheck(25);'+"`n"+'    u32 fogclr =').Replace('a_hex = (*(u32*)&a);','std::memcpy(&a_hex,&a,4);').Replace('c_hex = (*(u32*)&c);','std::memcpy(&c_hex,&c,4);').Replace('rgba = *(u32*)(&color);','rgba = u32(color.r)<<24|u32(color.g)<<16|u32(color.b)<<8|color.a;')
Set-Content "$n\src\generated\FogKernel.cpp" ('#include <revolution/gx.h>'+"`n"+$body)






