$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$s=Get-Content -Raw "$p\Petari\src\RVL_SDK\gx\GXFrameBuf.c"
$body=''
foreach($signature in @('void GXSetCopyClear(','void GXSetCopyFilter(')){
 $start=$s.IndexOf($signature);if($start -lt 0){throw 'Original display scaling anchor missing'};$brace=$s.IndexOf('{',$start);$depth=1;$end=$brace+1;while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++};$body+=$s.Substring($start,$end-$start)+"`n"
}
$body=$body.Replace('GX_WRITE_RA_REG(','GDWriteBPCmd(').Replace('BP_SENT();','')
$body=$body.Replace('    u32 reg;','    GDOverflowCheck(15);'+"`n"+'    u32 reg;').Replace('    u32 msLoc[4];','    GDOverflowCheck(30);'+"`n"+'    u32 msLoc[4];')
$body=@'
#include <revolution/gd/GDBase.h>
#include <private/gen_reg.h>
#include <private/pe_reg.h>
#define FAST_GPFLAGSET(reg,value,name) ((reg)=((reg)&~u32(name##_MASK))|((u32(value)<<name##_SHIFT)&u32(name##_MASK)))
#include <revolution/gx/shortcut_pe_reg.h>
#include <revolution/gx/shortcut_gen_reg.h>
'@+"
"+$body
Set-Content "$n\src\generated\DisplayCopyFilterKernel.cpp" ('#include <revolution/gx.h>'+"`n"+$body)




