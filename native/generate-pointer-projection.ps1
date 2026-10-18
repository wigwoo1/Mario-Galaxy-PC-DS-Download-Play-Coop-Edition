$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
$s=Get-Content "$p\Petari\src\Game\Util\DrawUtil.cpp" -Raw
$body="#include <Game/Util/DrawUtil.hpp>`n#include <Game/Util/ScreenUtil.hpp>`nnamespace { inline s32 getScreenHeightInline(){ return MR::getScreenHeight(); } }`n"
foreach($signature in @('J2DOrthoGraphSimple::J2DOrthoGraphSimple()', 'void J2DOrthoGraphSimple::setPort()')){
 $start=$s.IndexOf($signature);if($start -lt 0){throw 'Missing original projection method'}
 $brace=$s.IndexOf('{',$start);$end=$brace+1;$depth=1
 while($depth){if($s[$end] -eq '{'){$depth++}elseif($s[$end] -eq '}'){$depth--};$end++}
 $body+=$s.Substring($start,$end-$start)+"`n"
}
$start=$s.IndexOf('    void loadProjectionMtxFor2D()');$brace=$s.IndexOf('{',$start);$end=$s.IndexOf('}',$brace)+1
$body+="namespace MR {`n"+$s.Substring($start,$end-$start)+"`n}`n"
Set-Content "$n\src\generated\PointerProjection.cpp" $body
