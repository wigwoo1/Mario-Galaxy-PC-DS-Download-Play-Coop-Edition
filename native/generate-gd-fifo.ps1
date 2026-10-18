$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$destination="$n\include-runtime\JSystem\J3DGraphBase"
New-Item -ItemType Directory -Force $destination | Out-Null
$fifo=Get-Content "$p\Petari\libs\JSystem\include\JSystem\J3DGraphBase\J3DFifo.hpp" -Raw
foreach($pair in @(@('GXCmd1u8','GDWrite_u8'),@('GXCmd1u16','GDWrite_u16'),@('GXCmd1u32','GDWrite_u32'))){$fifo=$fifo.Replace($pair[0],$pair[1])}
foreach($pair in @(@('GXWGFifo.u8 = cmd;','GDWrite_u8(cmd);'),@('GXWGFifo.u16 = indx;','GDWrite_u16(indx);'),@('GXWGFifo.u16 = addr;','GDWrite_u16(addr);'))){
    if(!$fifo.Contains($pair[0])){throw 'FIFO indexed-write anchor missing'}
    $fifo=$fifo.Replace($pair[0],$pair[1])
}
Set-Content "$destination\J3DFifo.hpp" $fifo
$gd=Get-Content "$p\Petari\libs\JSystem\include\JSystem\J3DGraphBase\J3DGD.hpp" -Raw
$anchor='GXCmd1u32(*(u32*)data);'
if(!$gd.Contains($anchor)){throw 'FIFO float-write anchor missing'}
$gd=$gd.Replace($anchor,'GDWrite_f32(*data);').Replace('GXCmd1u32(fid.u);','GDWrite_u32(fid.u);')
Set-Content "$destination\J3DGD.hpp" $gd
$geometry=Get-Content "$p\Petari\src\RVL_SDK\gd\GDGeometry.c" -Raw
$end=$geometry.IndexOf('void GDSetArray(')
if($end -lt 0){throw 'original vertex descriptor boundary missing'}
Set-Content "$n\src\generated\GDVertexDescriptor.cpp" $geometry.Substring(0,$end)
$vertex=Get-Content "$p\Petari\libs\RVL_SDK\include\revolution\gx\GXVert.h" -Raw
$begin=$vertex.IndexOf('#ifdef __MWERKS__')
$finish=$vertex.IndexOf('#define __GXCDEF(')
if($begin -lt 0 -or $finish -le $begin){throw 'GX typed FIFO anchors missing'}
$replacement=@'
static inline void GDWrite_s8(s8 value) { GDWrite_u8((u8)value); }
static inline void GDWrite_s16(s16 value) { GDWrite_u16((u16)value); }

'@
$vertex=$vertex.Remove($begin,$finish-$begin).Insert($begin,$replacement+"`n")
foreach($component in @('x','y','z','w')){
    $vertex=$vertex.Replace("GXWGFifo.td = (td)$component;","GDWrite_##td((td)$component);")
}
Set-Content "$n\include-runtime\revolution\gx\GXVert.h" ('#include <revolution/gd/GDBase.h>'+"`n"+$vertex)

