$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\MathUtil.cpp" -Raw
$b=$s.IndexOf('void normalize(TVec2f* pVec)')
if($b -lt 0){throw '2D normalization anchor'}
$o=$s.IndexOf('{',$b);$d=1;$e=$o+1
while($d){if($s[$e] -eq '{'){$d++};if($s[$e] -eq '}'){$d--};$e++}
Set-Content "$n\src\generated\Vector2NormalizeKernel.cpp" ('#include <Game/Util/MathUtil.hpp>'+[Environment]::NewLine+'namespace MR {'+$s.Substring($b,$e-$b)+'}') -Encoding UTF8
