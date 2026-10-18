$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=[IO.File]::ReadAllText("$p\Petari\src\JSystem\J3DGraphAnimator\J3DModelData.cpp")
$b=$s.IndexOf('void J3DModelData::syncJ3DSysFlags() const');if($b -lt 0){throw 'Missing original model flags method'}
$o=$s.IndexOf('{',$b);$depth=1;$e=$o+1
while($depth){if($s[$e] -eq '{'){$depth++};if($s[$e] -eq '}'){$depth--};$e++}
$result='#include <JSystem/J3DGraphAnimator/J3DModelData.hpp>'+[Environment]::NewLine+'#include <JSystem/J3DGraphBase/J3DSys.hpp>'+[Environment]::NewLine+$s.Substring($b,$e-$b)
[IO.File]::WriteAllText("$n\src\generated\ModelFlagsKernel.cpp",$result)
