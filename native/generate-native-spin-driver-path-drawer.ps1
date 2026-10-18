$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\MapObj\SpinDriverPathDrawer.cpp" -Raw
$pattern='GXWGFifo\.f32 = rA1\.x;\s*GXWGFifo\.f32 = rA1\.y;\s*GXWGFifo\.f32 = rA1\.z;\s*GXWGFifo\.f32 = a2;\s*GXWGFifo\.f32 = a3;'
if(![regex]::IsMatch($s,$pattern)){throw 'Missing original position/UV FIFO sequence'}
$s=[regex]::Replace($s,$pattern,'Native::writePositionTexCoord(rA1.x, rA1.y, rA1.z, a2, a3);')
$s='#include <Native/GxImmediateVertices.hpp>' + "`n" + $s
Set-Content "$n\src\generated\SpinDriverPathDrawer.cpp" $s
