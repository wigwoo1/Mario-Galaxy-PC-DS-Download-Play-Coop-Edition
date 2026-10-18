$ErrorActionPreference='Stop'
$n=$PSScriptRoot;$p=Split-Path $n -Parent
function Extract([string]$source,[string]$signature){
 $b=$source.IndexOf($signature);if($b -lt 0){throw "Missing original accessor $signature"};$o=$source.IndexOf('{',$b);$d=1;$e=$o+1
 while($d){if($source[$e] -eq '{'){$d++}elseif($source[$e] -eq '}'){$d--};$e++};$source.Substring($b,$e-$b)
}
$access=Get-Content "$p\Petari\src\Game\Player\MarioAccess.cpp" -Raw
$util=Get-Content "$p\Petari\src\Game\Util\PlayerUtil.cpp" -Raw
$header=Get-Content "$p\Petari\include\Game\Player\MarioActor.hpp" -Raw
$anchor=$header.IndexOf('namespace NrvMarioActor {');if($anchor -lt 0){throw 'Missing original nerve declaration boundary'}
# Preserve complete original actor layout; exclude unrelated static nerve instances.
Set-Content "$n\src\generated\MarioActorAccess.hpp" $header.Substring(0,$anchor)
$parts=@('#include "MarioActorAccess.hpp"','#include <Game/Player/MarioHolder.hpp>','#include <Game/Player/MarioAccess.hpp>','#include <Game/Util/PlayerUtil.hpp>','namespace MarioAccess {',(Extract $access '    MarioActor* getPlayerActor()'),'}','namespace MR {',(Extract $util '    TVec3f* getPlayerPos()'),'}')
Set-Content "$n\src\generated\PlayerAccessKernel.cpp" ($parts -join [Environment]::NewLine)
