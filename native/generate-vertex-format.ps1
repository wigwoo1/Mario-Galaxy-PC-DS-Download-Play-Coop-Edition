$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$source=[IO.File]::ReadAllText("$p\Petari\src\RVL_SDK\gx\GXAttr.c")
$fields=[IO.File]::ReadAllText("$p\Petari\libs\RVL_SDK\include\private\cp_reg.h")
$start=$source.IndexOf('static inline void SETVAT(');$end=$source.IndexOf('void GXSetVtxAttrFmt(', $start)
$kernel=$source.Substring($start,$end-$start)
$kernel=[regex]::Replace($kernel,'SC_CP_VAT_REG_([ABC])_SET_(\w+)\((\*v[abc]), ([^)]+)\);',{
param($m)
$name='CP_VAT_REG_'+$m.Groups[1].Value+'_'+$m.Groups[2].Value
$shift=[regex]::Match($fields,'#define\s+'+$name+'_SHIFT\s+(\d+)').Groups[1].Value
$size=[regex]::Match($fields,'#define\s+'+$name+'_SIZE\s+(\d+)').Groups[1].Value
if(!$shift -or !$size){throw "Missing VAT field $name"}
return 'nativeVatField('+$m.Groups[3].Value+','+$m.Groups[4].Value+','+$shift+','+$size+');'
})
[IO.File]::WriteAllText("$n\src\generated\VatFieldKernel.inc",$kernel)
