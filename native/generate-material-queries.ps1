$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=[IO.File]::ReadAllText("$p\Petari\src\JSystem\J3DGraphLoader\J3DMaterialFactory.cpp")
$result='#include <JSystem/J3DGraphLoader/J3DMaterialFactory.hpp>'+[Environment]::NewLine
foreach($name in @('u16 J3DMaterialFactory::countUniqueMaterials(','u32 J3DMaterialFactory::countTexGens(','u32 J3DMaterialFactory::countStages(','const u8 J3DMaterialFactory::newColorChanNum(','u32 J3DMaterialFactory::newTexGenNum(','u8 J3DMaterialFactory::newCullMode(','u16 J3DMaterialFactory::newTexNo(','const u8 J3DMaterialFactory::newTevStageNum(','const u8 J3DMaterialFactory::newZCompLoc(','const u8 J3DMaterialFactory::newDither(')){$b=$s.IndexOf($name);if($b -lt 0){throw "Missing $name"};$o=$s.IndexOf('{',$b);$d=1;$e=$o+1;while($d){if($s[$e] -eq '{'){$d++};if($s[$e] -eq '}'){$d--};$e++};$result+=$s.Substring($b,$e-$b)+[Environment]::NewLine}
[IO.File]::WriteAllText("$n\src\generated\MaterialQueryKernel.cpp",$result)

