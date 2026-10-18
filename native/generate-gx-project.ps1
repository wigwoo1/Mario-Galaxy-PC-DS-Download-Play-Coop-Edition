$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\RVL_SDK\gx\GXTransform.c" -Raw
$a=$s.IndexOf('void GXProject(');$b=$s.IndexOf('// clang-format off',$a);if($a -lt 0 -or $b -lt 0){throw 'Missing original GXProject anchor'}
Set-Content "$n\src\generated\GXProjectKernel.cpp" ('#include <revolution/mtx.h>'+[Environment]::NewLine+'#include <revolution/gx/GXTransform.h>'+[Environment]::NewLine+'extern "C" '+$s.Substring($a,$b-$a))

