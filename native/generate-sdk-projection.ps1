$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\RVL_SDK\mtx\mtx44.c" -Raw
$end=$s.IndexOf('// clang-format off');if($end -lt 0){throw 'Original SDK projection boundary missing'}
$s=$s.Substring(0,$end).Replace('extern f64 tan(f64);','#include <cmath>')
Set-Content "$n\src\generated\SdkProjectionKernel.cpp" $s
