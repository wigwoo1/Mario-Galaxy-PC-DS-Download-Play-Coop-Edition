$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent;$s=Get-Content -Raw "$p\Petari\src\RVL_SDK\gx\GXFrameBuf.c"
$body=''
foreach($signature in @('void GXCopyDisp(')){
 $start=$s.IndexOf($signature);if($start -lt 0){throw 'Original display scaling anchor missing'};$brace=$s.IndexOf('{',$start);$depth=1;$end=$brace+1;while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++};$body+=$s.Substring($start,$end-$start)+"`n"
}
$body=$body.Replace('GX_WRITE_RA_REG(','GDWriteBPCmd(').Replace('BP_SENT();','')
$body=@'
#include <revolution/gd/GDBase.h>
#include <Native/DisplayCopyState.hpp>
#include <Native/GxCommandState.hpp>
#undef gx
#include <private/gen_reg.h>
#include <private/pe_reg.h>
#define FAST_GPFLAGSET(reg,value,name) ((reg)=((reg)&~u32(name##_MASK))|((u32(value)<<name##_SHIFT)&u32(name##_MASK)))
#include <revolution/gx/shortcut_pe_reg.h>
#include <revolution/gx/shortcut_gen_reg.h>
'@+"
"+$body
$body=$body.Replace('void GXCopyDisp(void* dest, GXBool clear)', 'void emitNativeDisplayCopy(void* dest, GXBool clear,const Native::GxCommandState& registers,Native::GpuResources& resources)')
$body=$body.Replace('    u32 reg;', '    struct State:Native::DisplayCopyState {u32 zmode,cmode0,peCtrl;};'+"`n"+'    State local{};static_cast<Native::DisplayCopyState&>(local)=Native::displayCopyState();local.zmode=0x40000000|registers.bp(0x40);local.cmode0=0x41000000|registers.bp(0x41);local.peCtrl=0x43000000|registers.bp(0x43);auto* gx=&local;'+"`n"+'    if(unsigned(clear)>1)throw std::invalid_argument("Texture copy clear flag");
    const auto nativeDestination=resources.address(dest);
    const bool changePixel=(clear || (gx->peCtrl&7)==3) && (gx->peCtrl&64);
    GDOverflowCheck(25+(clear?20:0)+(changePixel?10:0));'+"`n"+'    u32 reg;')
$body=$body.Replace('__GXVerifCopy(dest, clear);','').Replace('GX_PHY_ADDR(dest)','nativeDestination').Replace('FALSE','false').Replace('TRUE','true')
Set-Content "$n\src\generated\DisplayCopyEmitKernel.cpp" ('#include <revolution/gx.h>'+"`n"+$body)








