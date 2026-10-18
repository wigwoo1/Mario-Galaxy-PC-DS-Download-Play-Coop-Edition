$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\MathUtil.cpp" -Raw
$body=@('#include <Game/Util/MathUtil.hpp>','#include <Game/System/GameSystem.hpp>','#include <Game/System/GameSystemObjHolder.hpp>','#include <Game/Util/SingletonHolder.hpp>','#include <cstring>','namespace MR {')
foreach($sig in @('    f32 getRandom()','    f32 getRandom(f32','    s32 getRandom(s32','    f32 getRandomDegree(','    bool isHalfProbability(')){
 $a=$s.IndexOf($sig);if($a -lt 0){throw "Missing random anchor $sig"};$b=$s.IndexOf('{',$a);$d=1;$e=$b+1;while($d){if($s[$e] -eq '{'){$d++}elseif($s[$e] -eq '}'){$d--};$e++}
 $body+=$s.Substring($a,$e-$a).Replace('return reinterpret_cast< f32& >(value) - 1.0f;','f32 result; std::memcpy(&result, &value, sizeof(result)); return result - 1.0f;')
}
$body+='}'
Set-Content "$n\src\generated\RandomKernel.cpp" ($body -join [Environment]::NewLine)
